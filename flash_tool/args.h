#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_OPERATIONS (64)

enum device_state {
    DEVICE_STATE_NONE,
    DEVICE_STATE_PRELOADER,
    DEVICE_STATE_DA_STAGE2,
};

struct operation {
    int key;
    uint64_t address;
    uint64_t length;
    int fd;
};

struct arguments {
    enum device_state state;
    const char *download_agent;
    uint64_t address;
    uint64_t length;
    bool reboot;
    bool verbose;

    struct operation operations[MAX_OPERATIONS];
    size_t operations_count;

    int download_agent_fd;
};

void args_parse(int argc, char **argv, struct arguments *arguments);

#endif /* ARGS_H */
