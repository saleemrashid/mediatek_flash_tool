#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

void check_errnum(int errnum, const char *s);
void check_libusb(int errnum, const char *s);

void check_mtk_preloader(uint16_t status, const char *cmd);

void check_mtk_da_ack(uint8_t retval);
void check_mtk_da_cont_char(uint8_t retval);
void check_mtk_da_soc_ok(uint8_t retval);

#endif /* UTIL_H */
