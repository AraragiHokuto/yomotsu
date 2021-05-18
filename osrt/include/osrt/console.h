/* console.h -- general console interface */
#ifndef __RENZAN_LIBOS_CONSOLE_H__
#define __RENZAN_LIBOS_CONSOLE_H__

#include <stddef.h>

void console_write(const char* str, size_t str_size);

#endif	/* __RENZAN_LIBOS_CONSOLE_H__ */
