#include "mtk_da.h"

#include <errno.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>

#include <libusb.h>

#include "util.h"

int mtk_da_info_load(int fd, const mtk_da_info **info) {
    mtk_da_info tmp_info;
    if (read(fd, &tmp_info, sizeof(tmp_info)) != sizeof(tmp_info)) {
        return -errno;
    }

    if (tmp_info.da_info_magic != MTK_DA_INFO_MAGIC) {
        return -EINVAL;
    }
    if (tmp_info.da_info_ver != MTK_DA_INFO_VER) {
        return -EINVAL;
    }

    size_t length = offsetof(mtk_da_info, DA) + tmp_info.da_count * sizeof(mtk_da_entry);

    off_t maxlength;
    if ((maxlength = lseek(fd, 0, SEEK_END)) < 0) {
        return -errno;
    }

    if ((size_t) maxlength < length) {
        return -EFBIG;
    }

    const mtk_da_info *addr_info;
    if ((addr_info = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        return -errno;
    }

    *info = addr_info;

    return 0;
}

int mtk_da_sync(mtk_device *device, uint32_t *nand_ret, uint32_t *emmc_ret, uint32_t *emmc_id, uint8_t *da_major_ver, uint8_t *da_minor_ver) {
    int err;

    uint8_t sync_char;
    if ((err = mtk_device_read8(device, &sync_char)) < 0) {
        return err;
    }
    if (sync_char != MTK_DA_SYNC_CHAR) {
        return LIBUSB_ERROR_OTHER;
    }

    if ((err = mtk_device_read32(device, nand_ret)) < 0) {
        return err;
    }

    uint16_t nand_count;
    if ((err = mtk_device_read16(device, &nand_count)) < 0) {
        return err;
    }
    for (size_t i = 0; i < nand_count; i++) {
        if ((err = mtk_device_read16(device, NULL)) < 0) {
            return err;
        }
    }

    if ((err = mtk_device_read32(device, emmc_ret)) < 0) {
        return err;
    }
    for (size_t i = 0; i < 4; i++) {
        if ((err = mtk_device_read32(device, &emmc_id[i])) < 0) {
            return err;
        }
    }

    if ((err = mtk_device_write8(device, MTK_DA_ACK)) < 0) {
        return err;
    }

    if ((err = mtk_device_read8(device, da_major_ver)) < 0) {
        return err;
    }
    if ((err = mtk_device_read8(device, da_minor_ver)) < 0) {
        return err;
    }
    if ((err = mtk_device_read8(device, NULL)) < 0) {
        return err;
    }

    return 0;
}

static int send_device_config(mtk_device *device) {
    int err;

    if ((err = mtk_device_write8(device, 0xff)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 1)) < 0) {
        return err;
    }
    if ((err = mtk_device_write16(device, 0x0008)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 0x00)) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, 0x7007ffff)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 0x01)) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, 0)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 0x02)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 0x01)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 0x02)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, 0x00)) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, 1)) < 0) {
        return err;
    }

    return 0;
}

int mtk_da_send_da(mtk_device *device, uint32_t da_addr, uint32_t da_len, uint8_t *retval, const mtk_io_handler handler, void *user_data) {
    int err;

    if ((err = send_device_config(device)) < 0) {
        return err;
    }

    static const uint8_t name[16] = { 0x46, 0x46 };
    if ((err = mtk_device_write(device, name, sizeof(name))) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, 0xff000000)) < 0) {
        return err;
    }

    uint32_t data32;
    if ((err = mtk_device_read32(device, &data32)) < 0) {
        return err;
    }
    if (data32 != 0) {
        return LIBUSB_ERROR_OTHER;
    }

    uint8_t buffer[0x1000];

    if ((err = mtk_device_write32(device, da_addr)) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, da_len)) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, sizeof(buffer))) < 0) {
        return err;
    }

    if ((err = mtk_device_read8(device, retval)) < 0) {
        return err;
    }
    if (*retval != MTK_DA_ACK) {
        return 0;
    }

    size_t offset = 0;
    while (offset < da_len) {
        size_t count = MIN(sizeof(buffer), da_len - offset);

        if ((err = handler(true, offset, da_len, buffer, count, user_data)) < 0) {
            return err;
        }

        if ((err = mtk_device_write(device, buffer, count)) < 0) {
            return err;
        }

        offset += count;

        if ((err = mtk_device_read8(device, retval)) < 0) {
            return err;
        }
        if (*retval != MTK_DA_ACK) {
            return 0;
        }
    }

    if ((err = mtk_device_read8(device, retval)) < 0) {
        return err;
    }
    if (*retval != MTK_DA_ACK) {
        return 0;
    }

    if ((err = mtk_device_write8(device, MTK_DA_ACK)) < 0) {
        return err;
    }

    return 0;
}

int mtk_da_usb_check_status(mtk_device *device, uint8_t *usb_status, uint8_t *retval) {
    int err;

    if ((err = mtk_device_write8(device, MTK_DA_USB_CHECK_STATUS_CMD)) < 0) {
        return err;
    }

    if ((err = mtk_device_read8(device, retval)) < 0) {
        return err;
    }

    if (*retval == MTK_DA_ACK) {
        if ((err = mtk_device_read8(device, usb_status)) < 0) {
            return err;
        }
    }

    return 0;
}

int mtk_da_sdmmc_switch_part(mtk_device *device, uint8_t part, uint8_t *retval) {
    int err;

    if ((err = mtk_device_write8(device, MTK_DA_SWITCH_PART_CMD)) < 0) {
        return err;
    }
    if ((err = mtk_device_read8(device, retval)) < 0) {
        return err;
    }
    if (*retval == MTK_DA_ACK) {
        if ((err = mtk_device_write8(device, part)) < 0) {
            return err;
        }

        if ((err = mtk_device_read8(device, retval)) < 0) {
            return err;
        }
    }

    return 0;
}

int mtk_da_read(mtk_device *device, uint8_t hw_storage, uint64_t addr, uint64_t len, uint8_t *retval, const mtk_io_handler handler, void *user_data) {
    int err;

    if ((err = mtk_device_write8(device, MTK_DA_READ_CMD)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, MTK_DA_HOST_OS_LINUX)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, hw_storage)) < 0) {
        return err;
    }
    if ((err = mtk_device_write64(device, addr)) < 0) {
        return err;
    }
    if ((err = mtk_device_write64(device, len)) < 0) {
        return err;
    }

    if ((err = mtk_device_read8(device, retval)) < 0) {
        return err;
    }
    if (*retval != MTK_DA_ACK) {
        return 0;
    }

    uint8_t buffer[0x100000];
    if ((err = mtk_device_write32(device, sizeof(buffer))) < 0) {
        return err;
    }

    size_t offset = 0;
    while (offset < len) {
        size_t count = MIN(sizeof(buffer), len - offset);

        if ((err = mtk_device_read(device, buffer, count)) < 0) {
            return err;
        }

        uint16_t chksum = 0;
        for (size_t i = 0; i < count; i++) {
            chksum += buffer[i];
        }

        uint16_t chksum_device;
        if ((err = mtk_device_read16(device, &chksum_device)) < 0) {
            return err;
        }

        if (chksum != chksum_device) {
            return LIBUSB_ERROR_OTHER;
        }

        if ((err = mtk_device_write8(device, MTK_DA_ACK)) < 0) {
            return err;
        }

        if ((err = handler(false, offset, len, buffer, count, user_data)) < 0) {
            return err;
        }

        offset += count;
    }

    return 0;
}

int mtk_da_sdmmc_write_data(mtk_device *device, uint8_t storage_type, uint8_t part, uint64_t addr, uint64_t len, uint8_t *retval, const mtk_io_handler handler, void *user_data) {
    int err;

    if ((err = mtk_device_write8(device, MTK_DA_SDMMC_WRITE_DATA_CMD)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, storage_type)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, part)) < 0) {
        return err;
    }
    if ((err = mtk_device_write64(device, addr)) < 0) {
        return err;
    }
    if ((err = mtk_device_write64(device, len)) < 0) {
        return err;
    }

    uint8_t buffer[0x100000];
    if ((err = mtk_device_write32(device, sizeof(buffer))) < 0) {
        return err;
    }

    if ((err = mtk_device_read8(device, retval)) < 0) {
        return err;
    }
    if (*retval != MTK_DA_ACK) {
        return 0;
    }

    size_t offset = 0;
    while (offset < len) {
        if ((err = mtk_device_write8(device, MTK_DA_ACK)) < 0) {
            return 0;
        }

        size_t count = MIN(sizeof(buffer), len - offset);

        if ((err = handler(true, offset, len, buffer, count, user_data)) < 0) {
            return err;
        }

        if ((err = mtk_device_write(device, buffer, count)) < 0) {
            return err;
        }

        uint16_t chksum = 0;
        for (size_t i = 0; i < count; i++) {
            chksum += buffer[i];
        }

        if ((err = mtk_device_write16(device, chksum)) < 0) {
            return err;
        }

        if ((err = mtk_device_read8(device, retval)) < 0) {
            return err;
        }
        if (*retval != MTK_DA_CONT_CHAR) {
            return 0;
        }

        offset += count;
    }

    return 0;
}

int mtk_da_enable_watchdog(mtk_device *device, uint16_t timeout_ms, bool async, bool bootup, bool dlbit, bool not_reset_rtc_time, uint8_t *retval) {
    int err;

    if ((err = mtk_device_write8(device, MTK_DA_ENABLE_WATCHDOG_CMD)) < 0) {
        return err;
    }
    if ((err = mtk_device_write32(device, timeout_ms)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, async)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, bootup)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, dlbit)) < 0) {
        return err;
    }
    if ((err = mtk_device_write8(device, not_reset_rtc_time)) < 0) {
        return err;
    }
    if ((err = mtk_device_read8(device, retval))) {
        return err;
    }

    return 0;
}
