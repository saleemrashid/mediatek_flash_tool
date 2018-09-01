#include "mtk_preloader.h"

#include <stdbool.h>
#include <stddef.h>

#include <libusb.h>

#include "util.h"

int mtk_preloader_start(mtk_device *device) {
    static const uint8_t start_command[] = { 0xa0, 0x0a, 0x50, 0x05 };

    int err;

    if ((err = libusb_control_transfer(device->dev, LIBUSB_REQUEST_TYPE_CLASS, 0x20, 0, 0, NULL, 0, 0)) < 0) {
        return err;
    }

    size_t i = 0;
    while (i < sizeof(start_command)) {
        /* Ignore data read prior to start command */
        mtk_device_flush_buffer(device);

        if ((err = mtk_device_write8(device, start_command[i])) < 0) {
            return err;
        }

        uint8_t reply;
        if ((err = mtk_device_read8(device, &reply)) < 0) {
            return err;
        }

        uint8_t expected_reply = ~start_command[i];
        if (reply == expected_reply) {
            i++;
        } else {
            i = 0;
        }
    }

    return 0;
}

int mtk_preloader_get_tgt_config(mtk_device *device, uint32_t *tgt_config, uint16_t *status) {
    int err;

    if ((err = mtk_device_echo8(device, MTK_PRELOADER_CMD_GET_TARGET_CONFIG)) < 0) {
        return err;
    }
    if ((err = mtk_device_read32(device, tgt_config)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, status)) < 0) {
        return err;
    }

    return 0;
}

int mtk_preloader_get_hw_code(mtk_device *device, uint16_t *hw_code, uint16_t *status) {
    int err;

    if ((err = mtk_device_echo8(device, MTK_PRELOADER_CMD_GET_HW_CODE)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, hw_code)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, status)) < 0) {
        return err;
    }

    return 0;
}

int mtk_preloader_get_hw_sw_ver(mtk_device *device, uint16_t *hw_subcode, uint16_t *hw_ver, uint16_t *sw_ver, uint16_t *status) {
    int err;

    if ((err = mtk_device_echo8(device, MTK_PRELOADER_CMD_GET_HW_SW_VER)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, hw_subcode)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, hw_ver)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, sw_ver)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, status)) < 0) {
        return err;
    }

    return 0;
}

int mtk_preloader_write32(mtk_device *device, uint32_t base_addr, uint32_t len32, const uint32_t *data, uint16_t *status) {
    int err;

    if ((err = mtk_device_echo8(device, MTK_PRELOADER_CMD_WRITE32)) < 0) {
        return err;
    }
    if ((err = mtk_device_echo32(device, base_addr)) < 0) {
        return err;
    }
    if ((err = mtk_device_echo32(device, len32)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, status)) < 0) {
        return err;
    }

    if (*status == 0) {
        for (size_t i = 0; i < len32; i++) {
            if ((err = mtk_device_echo32(device, data[i])) < 0) {
                return err;
            }
        }
        if ((err = mtk_device_read16(device, status)) < 0) {
            return err;
        }
    }

    return 0;
}

int mtk_preloader_disable_wdt(mtk_device *device, uint16_t *status) {
    static const uint32_t data32 = 0x22000064;
    return mtk_preloader_write32(device, 0x10007000, 1, &data32, status);
}

int mtk_preloader_send_da(mtk_device *device, uint32_t da_addr, uint32_t da_len, uint32_t sig_len, uint16_t *status, const mtk_io_handler handler, void *user_data) {
    int err;

    if ((err = mtk_device_echo8(device, MTK_PRELOADER_CMD_SEND_DA)) < 0) {
        return err;
    }
    if ((err = mtk_device_echo32(device, da_addr)) < 0) {
        return err;
    }
    if ((err = mtk_device_echo32(device, da_len)) < 0) {
        return err;
    }
    if ((err = mtk_device_echo32(device, sig_len)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, status)) < 0) {
        return err;
    }

    if (*status == 0) {
        uint8_t buffer[0x400];
        uint16_t chksum = 0;

        size_t offset = 0;
        while (offset < da_len) {
            size_t count = MIN(sizeof(buffer), da_len - offset);

            if ((err = handler(true, offset, da_len, buffer, count, user_data)) < 0) {
                return err;
            }

            if ((err = mtk_device_write(device, buffer, count)) < 0) {
                return err;
            }

            for (size_t i = 0; i < (count >> 1); i++) {
                chksum ^= buffer[(i << 1)];
                chksum ^= buffer[(i << 1) + 1] << 8;
            }
            if (count & 1) {
                chksum ^= buffer[count - 1];
            }

            offset += count;
        }

        uint16_t chksum_device;
        if ((err = mtk_device_read16(device, &chksum_device)) < 0) {
            return err;
        }
        if ((err = mtk_device_read16(device, status)) < 0) {
            return err;
        }

        if (chksum != chksum_device) {
            return LIBUSB_ERROR_OTHER;
        }
    }

    return 0;
}

int mtk_preloader_jump_da(mtk_device *device, uint32_t da_addr, uint16_t *status) {
    int err;

    if ((err = mtk_device_echo8(device, MTK_PRELOADER_CMD_JUMP_DA)) < 0) {
        return err;
    }
    if ((err = mtk_device_echo32(device, da_addr)) < 0) {
        return err;
    }
    if ((err = mtk_device_read16(device, status)) < 0) {
        return err;
    }

    return 0;
}
