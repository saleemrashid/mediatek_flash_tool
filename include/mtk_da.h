#ifndef MTK_DA_H
#define MTK_DA_H

#include <stdbool.h>
#include <stdint.h>

#include "mtk_device.h"

#define MTK_DA_INFO_VER (0x4)
#define MTK_DA_INFO_MAGIC (0x22668899)

#define MTK_DA_ENTRY_MAGIC (0xdada)
#define MTK_DA_ENTRY_LOAD_REGIONS (10)

enum {
    MTK_DA_SOC_OK    = 0xc1,
    MTK_DA_SOC_FAIL  = 0xcf,
    MTK_DA_SYNC_CHAR = 0xc0,
    MTK_DA_CONT_CHAR = 0x69,
    MTK_DA_ACK       = 0x5a,
    MTK_DA_NACK      = 0xa5,
};

enum {
    MTK_DA_SWITCH_PART_CMD      = 0x60,
    MTK_DA_SDMMC_WRITE_DATA_CMD = 0x62,
    MTK_DA_USB_CHECK_STATUS_CMD = 0x72,
    MTK_DA_READ_CMD             = 0xd6,
    MTK_DA_ENABLE_WATCHDOG_CMD  = 0xdb,
};

#define MTK_DA_NAND_NOT_FOUND  (0xbc4)

#define MTK_DA_FULL_REPORT_SIZE (235)

enum {
    MTK_DA_HW_STORAGE_NOR = 0,
    MTK_DA_HW_STORAGE_NAND,
    MTK_DA_HW_STORAGE_EMMC,
    MTK_DA_HW_STORAGE_SDMMC,
    MTK_DA_HW_STORAGE_UFS,
};

enum {
    MTK_DA_STORAGE_EMMC = 0x1,
    MTK_DA_STORAGE_SDMMC,
};

enum {
    MTK_DA_EMMC_PART_BOOT1 = 1,
    MTK_DA_EMMC_PART_BOOT2,
    MTK_DA_EMMC_PART_RPMB,
    MTK_DA_EMMC_PART_GP1,
    MTK_DA_EMMC_PART_GP2,
    MTK_DA_EMMC_PART_GP3,
    MTK_DA_EMMC_PART_GP4,
    MTK_DA_EMMC_PART_USER,
};

enum {
    MTK_DA_HOST_OS_MAC = 10,
    MTK_DA_HOST_OS_WINDOWS,
    MTK_DA_HOST_OS_LINUX,
};

typedef struct {
    uint32_t offset;
    uint32_t len;
    uint32_t start_addr;
    uint32_t sig_offset;
    uint32_t sig_len;
} __attribute__((packed)) mtk_da_load_region;

typedef struct {
    uint16_t magic;
    uint16_t hw_code;
    uint16_t hw_sub_code;
    uint16_t hw_ver;
    uint16_t sw_ver;
    uint16_t chip_evolution;
    uint32_t da_feature_set;
    uint16_t entry_region_index;
    uint16_t load_regions_count;
    mtk_da_load_region load_regions[MTK_DA_ENTRY_LOAD_REGIONS];
} __attribute__((packed)) mtk_da_entry;

typedef struct {
    char da_identifier[32];
    char da_description[64];
    uint32_t da_info_ver;
    uint32_t da_info_magic;
    uint32_t da_count;
    mtk_da_entry DA[];
} __attribute__((packed)) mtk_da_info;

int mtk_da_info_load(int fd, const mtk_da_info **info);

int mtk_da_sync(mtk_device *device, uint32_t *nand_ret, uint32_t *emmc_ret, uint32_t *emmc_id, uint8_t *da_major_ver, uint8_t *da_minor_ver);
int mtk_da_send_da(mtk_device *device, uint32_t da_addr, uint32_t da_len, uint8_t *retval, const mtk_io_handler handler, void *user_data);

int mtk_da_usb_check_status(mtk_device *device, uint8_t *usb_status, uint8_t *retval);

int mtk_da_sdmmc_switch_part(mtk_device *device, uint8_t part, uint8_t *retval);
int mtk_da_sdmmc_write_data(mtk_device *device, uint8_t storage_type, uint8_t part, uint64_t addr, uint64_t len, uint8_t *retval, const mtk_io_handler handler, void *user_data);
int mtk_da_read(mtk_device *device, uint8_t hw_storage, uint64_t addr, uint64_t len, uint8_t *retval, const mtk_io_handler handler, void *user_data);

int mtk_da_enable_watchdog(mtk_device *device, uint16_t timeout_ms, bool async, bool reboot, bool download_mode, bool no_reset_rtc_time, uint8_t *retval);

#endif /* MTK_DA_H */
