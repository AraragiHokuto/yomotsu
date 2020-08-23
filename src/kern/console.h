#ifndef IZANAMI_CONSOLE_H_
#define IZANAMI_CONSOLE_H_

#include <kern/types.h>

#ifdef _KERNEL

enum { CON_COLOR_BLACK         = 0,
       CON_COLOR_BLUE          = 1,
       CON_COLOR_GREEN         = 2,
       CON_COLOR_CYAN          = 3,
       CON_COLOR_RED           = 4,
       CON_COLOR_MANGENTA      = 5,
       CON_COLOR_BROWN         = 6,
       CON_COLOR_LIGHT_GRAY    = 7,
       CON_COLOR_DARK_GRAY     = 8,
       CON_COLOR_LIGHT_BLUE    = 9,
       CON_COLOR_LIGHT_GREEN   = 10,
       CON_COLOR_LIGHT_CYAN    = 11,
       CON_COLOR_LIGHT_RED     = 12,
       CON_COLOR_LIGHT_MAGENTA = 13,
       CON_COLOR_YELLOW        = 14,
       CON_COLOR_WHITE         = 15 };

typedef struct con_driver_s {
        uint (*dim_x)();
        uint (*dim_y)();
        void (*write)(uint fg, uint bg, const char *str, size_t len);
        void (*scroll)(uint lc);
        void (*clear)();
} con_driver_t;

void con_init(void);
void con_set_driver(con_driver_t *driver);

uint con_dim_x(void);
uint con_dim_y(void);
void con_write(uint fg, uint bg, const char *str, size_t len);
void con_scroll(uint lc);
void con_clear(void);

void kprintf(const char *fmt, ...);
void kvprintf(__builtin_va_list ap, const char *mt);

#endif /* _KERNEL */

#endif /* IZANAMI_CONSOLE_H_ */
