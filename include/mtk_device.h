#ifndef MTK_DEVICE_H
#define MTK_DEVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libusb.h>

#define MTK_DEVICE_PKTSIZE (512)

#define MTK_DEVICE_TMOUT (1000)

#define MTK_DEVICE_INTERFACE (0)

#define MTK_DEVICE_EPIN  (0x1 | LIBUSB_ENDPOINT_IN)
#define MTK_DEVICE_EPOUT (0x1 | LIBUSB_ENDPOINT_OUT)

#define MTK_DEVICE_VID (0x0e8d)
#define MTK_DEVICE_PID (0x2000)

typedef struct {
    libusb_device_handle *dev;

    uint8_t buffer[MTK_DEVICE_PKTSIZE];
    size_t buffer_available;
    size_t buffer_offset;
} mtk_device;

typedef int (*mtk_io_handler)(bool, size_t, size_t, uint8_t *, size_t, void *);

int mtk_device_open(mtk_device *device, libusb_device_handle *dev);

int mtk_device_detect(mtk_device *device, libusb_context *ctx);

int mtk_device_read(mtk_device *device, uint8_t *buffer, size_t size);
int mtk_device_write(mtk_device *device, const uint8_t *buffer, size_t size);

static inline void mtk_device_flush_buffer(mtk_device *device) {
    device->buffer_available = 0;
}

int mtk_device_read8(mtk_device *device, uint8_t *data);
int mtk_device_read16(mtk_device *device, uint16_t *data);
int mtk_device_read32(mtk_device *device, uint32_t *data);
int mtk_device_read64(mtk_device *device, uint64_t *data);

int mtk_device_write8(mtk_device *device, uint8_t data);
int mtk_device_write16(mtk_device *device, uint16_t data);
int mtk_device_write32(mtk_device *device, uint32_t data);
int mtk_device_write64(mtk_device *device, uint64_t data);

int mtk_device_echo8(mtk_device *device, uint8_t data);
int mtk_device_echo16(mtk_device *device, uint16_t data);
int mtk_device_echo32(mtk_device *device, uint32_t data);
int mtk_device_echo64(mtk_device *device, uint64_t data);

#endif /* MTK_DEVICE_H */
