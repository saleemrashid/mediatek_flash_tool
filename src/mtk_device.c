#include "mtk_device.h"

#include <endian.h>
#include <stdbool.h>
#include <string.h>

#include <libusb.h>

#include "util.h"

int mtk_device_open(mtk_device *device, libusb_device_handle *dev) {
    device->dev = dev;
    device->buffer_offset = 0;
    device->buffer_available = 0;

    int err;

    if ((err = libusb_set_auto_detach_kernel_driver(dev, true)) < 0) {
        return err;
    }

    if ((err = libusb_claim_interface(dev, MTK_DEVICE_INTERFACE)) < 0) {
        return err;
    }

    if ((err = libusb_claim_interface(dev, MTK_DEVICE_INTERFACE)) < 0) {
        return err;
    }

    return 0;
}

static int hotplug_callback_fn(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data) {
    (void) ctx;
    (void) event;

    libusb_device **dev = user_data;
    *dev = device;

    return true;
}

int mtk_device_detect(mtk_device *device, libusb_context *ctx) {
    libusb_device *dev = NULL;

    int err = libusb_hotplug_register_callback(ctx,
        LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
        LIBUSB_HOTPLUG_ENUMERATE,
        MTK_DEVICE_VID,
        MTK_DEVICE_PID,
        LIBUSB_CLASS_COMM,
        hotplug_callback_fn,
        &dev,
        NULL);

    if (err < 0) {
        return err;
    }

    while (dev == NULL) {
        libusb_handle_events(ctx);
    }

    libusb_device_handle *devh;
    if ((err = libusb_open(dev, &devh)) < 0) {
        return err;
    }

    return mtk_device_open(device, devh);
}

int mtk_device_read(mtk_device *device, uint8_t *buffer, size_t size) {
    size_t offset = 0;

    while (offset < size) {
        if (device->buffer_available == 0) {
            int transferred;

            int err;
            if ((err = libusb_bulk_transfer(device->dev, MTK_DEVICE_EPIN, device->buffer, MTK_DEVICE_PKTSIZE, &transferred, MTK_DEVICE_TMOUT)) < 0) {
                return err;
            }

            device->buffer_offset = 0;
            device->buffer_available = transferred;
        }

        size_t n = MIN(size - offset, device->buffer_available);
        if (buffer != NULL) {
            memcpy(buffer + offset, device->buffer + device->buffer_offset, n);
        }

        offset += n;
        device->buffer_offset += n;
        device->buffer_available -= n;
    }

    return 0;
}

int mtk_device_write(mtk_device *device, const uint8_t *buffer, size_t size) {
    size_t offset = 0;

    while (offset < size) {
        int transferred;

        int err = libusb_bulk_transfer(device->dev, MTK_DEVICE_EPOUT, (uint8_t *) buffer + offset, size - offset, &transferred, MTK_DEVICE_TMOUT);
        if (err < 0) {
            return err;
        }

        offset += transferred;
    }

    return 0;
}

int mtk_device_read8(mtk_device *device, uint8_t *data) {
    return mtk_device_read(device, data, sizeof(*data));
}

int mtk_device_read16(mtk_device *device, uint16_t *data) {
    int err = mtk_device_read(device, (uint8_t *) data, sizeof(*data));
    if (err < 0) {
        return err;
    }
    if (data != NULL) {
        *data = be16toh(*data);
    }
    return 0;
}

int mtk_device_read32(mtk_device *device, uint32_t *data) {
    int err = mtk_device_read(device, (uint8_t *) data, sizeof(*data));
    if (err < 0) {
        return err;
    }
    if (data != NULL) {
        *data = be32toh(*data);
    }
    return 0;
}

int mtk_device_read64(mtk_device *device, uint64_t *data) {
    int err = mtk_device_read(device, (uint8_t *) data, sizeof(*data));
    if (err < 0) {
        return err;
    }
    if (data != NULL) {
        *data = be64toh(*data);
    }
    return 0;
}

int mtk_device_write8(mtk_device *device, uint8_t data) {
    return mtk_device_write(device, &data, sizeof(data));
}

int mtk_device_write16(mtk_device *device, uint16_t data) {
    data = htobe16(data);
    return mtk_device_write(device, (uint8_t *) &data, sizeof(data));
}

int mtk_device_write32(mtk_device *device, uint32_t data) {
    data = htobe32(data);
    return mtk_device_write(device, (uint8_t *) &data, sizeof(data));
}

int mtk_device_write64(mtk_device *device, uint64_t data) {
    data = htobe64(data);
    return mtk_device_write(device, (uint8_t *) &data, sizeof(data));
}

int mtk_device_echo8(mtk_device *device, uint8_t data) {
    int err;

    if ((err = mtk_device_write8(device, data)) < 0) {
        return err;
    }

    uint8_t reply;
    if ((err = mtk_device_read8(device, &reply)) < 0) {
        return err;
    }
    if (reply != data) {
        return LIBUSB_ERROR_OTHER;
    }

    return 0;
}

int mtk_device_echo16(mtk_device *device, uint16_t data) {
    int err;

    if ((err = mtk_device_write16(device, data)) < 0) {
        return err;
    }

    uint16_t reply;
    if ((err = mtk_device_read16(device, &reply)) < 0) {
        return err;
    }
    if (reply != data) {
        return LIBUSB_ERROR_OTHER;
    }

    return 0;
}

int mtk_device_echo32(mtk_device *device, uint32_t data) {
    int err;

    if ((err = mtk_device_write32(device, data)) < 0) {
        return err;
    }

    uint32_t reply;
    if ((err = mtk_device_read32(device, &reply)) < 0) {
        return err;
    }
    if (reply != data) {
        return LIBUSB_ERROR_OTHER;
    }

    return 0;
}

int mtk_device_echo64(mtk_device *device, uint64_t data) {
    int err;

    if ((err = mtk_device_write64(device, data)) < 0) {
        return err;
    }

    uint64_t reply;
    if ((err = mtk_device_read64(device, &reply)) < 0) {
        return err;
    }
    if (reply != data) {
        return LIBUSB_ERROR_OTHER;
    }

    return 0;
}
