#ifndef MTK_PRELOADER_H
#define MTK_PRELOADER_H

#include "mtk_device.h"

#include <stdint.h>

enum {
    MTK_PRELOADER_CMD_GET_HW_SW_VER     = 0xfc,
    MTK_PRELOADER_CMD_GET_HW_CODE       = 0xfd,

    MTK_PRELOADER_CMD_WRITE32           = 0xd4,
    MTK_PRELOADER_CMD_JUMP_DA           = 0xd5,
    MTK_PRELOADER_CMD_SEND_DA           = 0xd7,
    MTK_PRELOADER_CMD_GET_TARGET_CONFIG = 0xd8,
};

int mtk_preloader_start(mtk_device *device);

int mtk_preloader_get_tgt_config(mtk_device *device, uint32_t *tgt_config, uint16_t *status);

int mtk_preloader_get_hw_code(mtk_device *device, uint16_t *hw_code, uint16_t *status);
int mtk_preloader_get_hw_sw_ver(mtk_device *device, uint16_t *hw_subcode, uint16_t *hw_ver, uint16_t *sw_ver, uint16_t *status);

int mtk_preloader_write32(mtk_device *device, uint32_t base_addr, uint32_t len32, const uint32_t *data, uint16_t *status);

int mtk_preloader_disable_wdt(mtk_device *device, uint16_t *status);

int mtk_preloader_send_da(mtk_device *device, uint32_t da_addr, uint32_t da_len, uint32_t sig_len, uint16_t *status, const mtk_io_handler handler, void *user_data);
int mtk_preloader_jump_da(mtk_device *device, uint32_t da_addr, uint16_t *status);

#endif /* MTK_PRELOADER_H */
