#include "io_handler.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PROGRESS_BAR_WIDTH (48)
#define SI_UNITS_BUFSIZ (16)

static void print_progress(bool flashing, size_t offset, size_t length);
static void format_si_units(size_t length, char *str, size_t size);

int io_handler(bool flashing, size_t offset, size_t total_length, uint8_t *buffer, size_t count, void *user_data) {
    const struct file_info *fi = user_data;

    if (lseek(fi->fd, fi->offset + offset, SEEK_SET) < 0) {
        errx(1, "Unable to seek file descriptor: %s", strerror(errno));
    }

    if (flashing) {
        ssize_t n;
        if ((n = read(fi->fd, buffer, count)) < 0) {
            errx(1, "Unable to read from file descriptor: %s", strerror(errno));
        }
        if ((size_t) n != count) {
            errx(1, "Not enough data read from file descriptor");
        }
    } else {
        ssize_t n;
        if ((n = write(fi->fd, buffer, count)) < 0) {
            errx(1, "Unable to write to file descriptor: %s", strerror(errno));
        }
        if ((size_t) n != count) {
            errx(1, "Not enough data written to file descriptor");
        }
    }

    print_progress(flashing, offset + count, total_length);
    return 0;
}

static void print_progress(bool flashing, size_t offset, size_t length) {
    if (!isatty(STDERR_FILENO)) {
        return;
    }

    double progress = (double) offset / length;

    char progress_bar[PROGRESS_BAR_WIDTH + 1];

    size_t progress_bar_fill = progress * PROGRESS_BAR_WIDTH;
    memset(progress_bar, '#', progress_bar_fill);
    memset(progress_bar + progress_bar_fill, '-', PROGRESS_BAR_WIDTH - progress_bar_fill);
    progress_bar[PROGRESS_BAR_WIDTH] = '\0';

    const char *verb = flashing ? "Flashing" : "Dumping";
    int percent = progress * 100;

    char offset_str[SI_UNITS_BUFSIZ];
    format_si_units(offset, offset_str, sizeof(offset_str));
    char length_str[SI_UNITS_BUFSIZ];
    format_si_units(length, length_str, sizeof(length_str));

    fprintf(stderr, "%s %-8s of %-8s  [%s]  %3d%%%c", verb, offset_str, length_str, progress_bar, percent, (offset == length) ? '\n' : '\r');
    fflush(stderr);
}

static void format_si_units(size_t length, char *str, size_t size) {
    static const char *suffix = "BKMG";

    double dbl = length;
    const char *ptr = suffix;

    while (dbl > 1024 && *(ptr + 1) != '\0') {
        dbl /= 1024;
        ptr++;
    }

    snprintf(str, size, "%.5g %c", dbl, *ptr);
}
