/* k_console.h -- Debugging console output */

#ifndef __RENZAN_K_CONSOLE_H__
#define __RENZAN_K_CONSOLE_H__

#include <k_cdefs.h>

#include <osrt/types.h>

#ifdef _KDEBUG

void con_init(void);
void con_write(const char *str, size_t len);

void kprintf(const char *fmt, ...);
void kvprintf(__builtin_va_list ap, const char *mt);

#else /* _KDEBUG */

static void
con_init(void)
{
}

static void
con_write(const char *str, size_t len)
{
	DONTCARE(str);
	DONTCARE(len);
}

static void
kprintf(const char *fmt, ...)
{
        DONTCARE(fmt);
}

static void
kvprintf(__builtin_va_list ap, const char *mt)
{
        DONTCARE(ap);
        DONTCARE(mt);
}

#endif /* _KDEBUG */

#endif /* __RENZAN_K_CONSOLE_H__ */
