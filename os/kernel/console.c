#include <os/kernel/boolean.h>
#include <os/kernel/console.h>
#include <os/kernel/interrupt.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/spinlock.h>
#include <os/kernel/string.h>
#include <os/kernel/types.h>

#include <hal_percpu.h>

static con_driver_t *_driver;
static spinlock_t    lock;

void
con_init(void)
{
        spinlock_init(&lock);
}

void
con_set_driver(con_driver_t *driver)
{
        ASSERT(driver);
        _driver = driver;
}

uint
con_dim_x(void)
{
        ASSERT(_driver);
        ASSERT(_driver->dim_x);
        return _driver->dim_x();
}

uint
con_dim_y(void)
{
        ASSERT(_driver);
        ASSERT(_driver->dim_y);
        return _driver->dim_y();
}

void
con_write(uint fg, uint bg, const char *str, size_t len)
{
        ASSERT(_driver);
        ASSERT(_driver->write);

        INTERRUPT_CRITICAL_BEGIN
                {
                        _driver->write(fg, bg, str, len);
                }
        INTERRUPT_CRITICAL_END;
}

void
con_scroll(uint lc)
{
        ASSERT(_driver);
        ASSERT(_driver->scroll);

        INTERRUPT_CRITICAL_BEGIN
                {
                        _driver->scroll(lc);
                }
        INTERRUPT_CRITICAL_END;
}

void
con_clear()
{
        ASSERT(_driver);
        ASSERT(_driver->clear);

        INTERRUPT_CRITICAL_BEGIN
                {
                        _driver->clear();
                }
        INTERRUPT_CRITICAL_END;
}

static size_t
__itoa(char *buf, uint i, uint base)
{
        /* This routine is not intended for arbitrary base */
        ASSERT(base == 10 || base == 16);

        if (i == 0 && base == 10) {
                *buf = '0';
                return 1;
        }

        char *p = buf;

        const char *alphabet = "0123456789abcdef";

        /* convert integer to reversed string */
        while (i != 0) {
                *p++ = alphabet[i % base];
                i /= base;
        }

        /* reverse string */
        char *end;
        if (base == 16) {
                end = buf + 16 - 1;
                kmemset(p, '0', end - p + 1);
        } else {
                end = p - 1;
        }

        size_t ret = end - buf + 1;

        while (end > buf) {
                char c = *end;
                *end   = *buf;
                *buf   = c;
                end--;
                buf++;
        }

        return ret;
}

/*
 * Kernel implementation of printf-like function.
 * Note that this function uses different notation from stdc printf:
 * %%		-- '%' character
 * %s		-- null-terminated string
 * %sn		-- const char * pointer followed by a size_t
 *		   argument to specify length
 * %x		-- uint	formatted as hex string
 * %d		-- uint formatted as dec string
 * %[xd][bslq]	-- different length of uint formatted as hex or dec string
 * %S[xd][bslq]	-- different length of sint formatted as hex or dec string
 */
void
kprintf(const char *fmt, ...)
{
        __builtin_va_list ap;
        __builtin_va_start(ap, fmt);

        return kvprintf(ap, fmt);
}

void
kvprintf(__builtin_va_list ap, const char *fmt)
{
        const char *rs = fmt;
        const char *p  = fmt;

        u8 fg = CON_COLOR_LIGHT_GRAY;
        u8 bg = CON_COLOR_BLACK;

        irqflag_t flag;
        spinlock_lock(&lock, &flag);

        while (*p != '\0') {
                if (*p != '%') {
                        ++p;
                        continue;
                }

                /* *p == '%' */
                con_write(fg, bg, rs, p - rs);
                ++p;

                switch (*p) {
                case '\0':
                        continue;
                case '%':
                        rs = p++;
                        continue;
                case 's':
                        goto fmt_s;
                case 'x':
                case 'd':
                case 'S':
                        goto fmt_int;
                default:
                        rs = ++p;
                        continue;
                }

fmt_s:
                ++p;
                const char *str = __builtin_va_arg(ap, const char *);
                size_t      ss;
                if (*p == 'n') {
                        ss = __builtin_va_arg(ap, size_t);
                        ++p;
                } else {
                        ss = kstrlen(KSTRLEN_MAX_SIZE_UNKNOWN, str);
                }

                con_write(fg, bg, str, ss);
                rs = p;
                continue;

fmt_int:;
                /* int format parsing */
                uint value; /* absolute value of integer */
                /* size of integer. can be 1, 2, 4 or 8 */
                size_t  size    = 8;
                uint    base    = 10;      /* base of output */
                boolean signess = B_FALSE; /* whether integer is signed */

                if (*p == 'S') {
                        ++p;
                        signess = B_TRUE;
                }

                switch (*p) {
                case 'x':
                        base = 16;
                        ++p;
                        break;
                case 'd':
                        base = 10;
                        ++p;
                        break;
                default:
                        /* only if %S */
                        goto output_int;
                }

                switch (*p) {
                case 'b':
                        size = 1;
                        ++p;
                        break;
                case 's':
                        size = 2;
                        ++p;
                        break;
                case 'l':
                        size = 4;
                        ++p;
                        break;
                case 'q':
                        size = 8;
                        ++p;
                        break;
                default:
                        break;
                }

output_int:
                if (signess) {
                        sint sv;
                        if (size <= 4) {
                                sv = __builtin_va_arg(ap, int);
                        } else {
                                sv = __builtin_va_arg(ap, s64);
                        }
                        if (sv < 0) {
                                value = -sv;
                                con_write(fg, bg, "-", 1);
                        } else {
                                value = sv;
                        }
                } else {
                        if (size <= 4) {
                                value = __builtin_va_arg(ap, unsigned int);
                        } else {
                                value = __builtin_va_arg(ap, u64);
                        }
                }

                char   buf[64];
                size_t output_len = __itoa(buf, value, base);
                con_write(fg, bg, buf, output_len);
                rs = p;
                continue;
        }

        if (p != rs) { con_write(fg, bg, rs, p - rs); }

        spinlock_unlock(&lock, flag);
}
