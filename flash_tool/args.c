#include "args.h"

#include <argp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static error_t parse_opt(int key, char *arg, struct argp_state *state);
static uint64_t parse_uint64_opt(int key, const char *str, const struct argp_state *state);

static const struct argp_option options[] = {
    { "da-stage2",      '2',  NULL,     0, "Device is in DA Stage 2", 0 },
    { "preloader",      'P',  NULL,     0, "Device is in Preloader mode", 0 },
    { "download-agent", 'd', "FILE",    0, "Path to MediaTek Download Agent binary", 1 },
    { "address",        'a', "ADDRESS", 0, "EMMC address to read/write", 2 },
    { "length",         'l', "LENGTH",  0, "Length of data to read/write", 2 },
    { "dump",           'D', "FILE",    0, "Path to dump data to", 3 },
    { "flash",          'F', "FILE",    0, "Path to flash data from", 3 },
    { "reboot",         'R',  NULL,     0, "Reboot device after completion", 4 },
    { "verbose",        'v',  NULL,     0, "Produce verbose output", -1 },
    {  NULL,             0,   NULL,     0,  NULL, 0 },
};

static const struct argp argp = {
    .options = options,
    .parser = parse_opt,

    .args_doc = NULL,
    .doc = NULL,
    .children = NULL,
    .help_filter = NULL,
    .argp_domain = NULL,
};

void args_parse(int argc, char **argv, struct arguments *arguments) {
    argp_parse(&argp, argc, argv, 0, NULL, arguments);
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    bool flashing;

    switch (key) {
        case ARGP_KEY_INIT:
            arguments->state = DEVICE_STATE_NONE;
            arguments->download_agent = NULL;
            arguments->address = 0;
            arguments->length = 0;
            arguments->reboot = false;
            arguments->verbose = false;
            arguments->operations_count = 0;
            arguments->download_agent_fd = -1;
            break;

        case '2':
            arguments->state = DEVICE_STATE_DA_STAGE2;
            break;
        case 'P':
            arguments->state = DEVICE_STATE_PRELOADER;
            break;
        case 'd':
            arguments->download_agent = arg;
            break;
        case 'a':
            arguments->address = parse_uint64_opt(key, arg, state);
            break;
        case 'l':
            arguments->length = parse_uint64_opt(key, arg, state);
            break;
        case 'R':
            arguments->reboot = true;
            break;
        case 'v':
            arguments->verbose = true;
            break;

        case 'D':
        case 'F':
            flashing = (key == 'F');

            if (arguments->operations_count == MAX_OPERATIONS) {
                argp_error(state, "Too many operations");
            }
            if (arguments->length == 0) {
                argp_error(state, "Cannot perform zero-length operation");
            }

            struct operation *operation = &arguments->operations[arguments->operations_count++];

            operation->key = key;
            operation->address = arguments->address;
            operation->length = arguments->length;

            int flags;
            const char *verb;

            if (flashing) {
                flags = O_RDONLY;
                verb = "flashing";
            } else {
                flags = O_WRONLY | O_CREAT | O_TRUNC;
                verb = "dumping";
            }

            if ((operation->fd = open(arg, flags, DEFFILEMODE)) < 0) {
                argp_failure(state, 1, errno, "Unable to open file for %s: %s", verb, arg);
            }

            if (flashing) {
                off_t maxlength;
                if ((maxlength = lseek(operation->fd, 0, SEEK_END)) < 0) {
                    argp_failure(state, 1, errno, "Unable to seek file descriptor: %s", arg);
                }
                if ((uint64_t) maxlength < arguments->length) {
                    argp_failure(state, 1, 0, "Write length is greater than file size: %s", arg);
                }
            }
            break;

        case ARGP_KEY_ARG:
            argp_usage(state);
            break;

        case ARGP_KEY_END:
            if (arguments->state != DEVICE_STATE_DA_STAGE2) {
                if (arguments->download_agent == NULL) {
                    argp_error(state, "MediaTek Download Agent binary is mandatory, unless device is in DA Stage 2");
                }
                if ((arguments->download_agent_fd = open(arguments->download_agent, O_RDONLY)) < 0) {
                    argp_failure(state, 1, errno, "Unable to open Download Agent binary");
                }
            }
            break;
    }

    return 0;
}

static uint64_t parse_uint64_opt(int key, const char *str, const struct argp_state *state) {
    if (isdigit(*str)) {
        char *endptr = NULL;

        _Static_assert(ULLONG_MAX == UINT64_MAX, "unsigned long long is incompatible with uint64_t");

        errno = 0;
        uint64_t value = strtoull(str, &endptr, 0);

        if (errno == 0 && *endptr == '\0') {
            return value;
        }
    }

    argp_error(state, "option requires an integer -- '%c'", key);
    return 0;
}
