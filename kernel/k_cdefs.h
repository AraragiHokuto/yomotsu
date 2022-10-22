#ifndef __RENZAN_K_CDEFS_H__
#define __RENZAN_K_CDEFS_H__

#include <osrt/types.h>

#define NULL ((void *)0)

#define DONTCARE(x) ((void)(x))

/* attributes */
#define PACKED       __attribute__((packed))
#define FORCE_INLINE __attribute__((always_inline))
#define NORETURN     __attribute__((noreturn))

/* branch hints */
#define LIKELY(x)   __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)

/*
 * I know this identifier is reserved for implementation.
 * We are the implementation.
 */
void __panic(
    const char *file, uint line, const char *func, const char *fmt,
    ...) NORETURN;

#define PANIC(fmt, ...) \
        __panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define UNREACHABLE PANIC("Reached unreachable")

#ifdef _KDEBUG
#define ASSERT(x)                                                \
        do {                                                     \
                if (!(x)) { PANIC("assertion failed: %s", #x); } \
        } while (0)
#else /* _KDEBUG */
#define ASSERT(x) DONTCARE(x)
#endif /* _KDEBUG */

#define VERIFY(x, fmt, ...)                              \
        do {                                             \
                if (!(x)) { PANIC(fmt, ##__VA_ARGS__); } \
        } while (0)

#endif /* __RENZAN_K_CDEFS_H__ */
