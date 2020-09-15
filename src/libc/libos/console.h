/* console.h -- general console interface */
/* WARNING unstable interface */
#ifndef __ORIHIME_LIBOS_CONSOLE_H__
#define __ORIHIME_LIBOS_CONSOLE_H__

#include <stddef.h>

void console_write(const char* str, size_t str_size);

#endif	/* __ORIHIME_LIBOS_CONSOLE_H__ */
