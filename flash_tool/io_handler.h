#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct file_info {
    int fd;
    size_t offset;
};

int io_handler(bool flashing, size_t offset, size_t total_length, uint8_t *buffer, size_t count, void *user_data);

#endif /* IO_HANDLER_H */
