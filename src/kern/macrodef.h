#ifndef YOMOTSU_MACRODEF_H_
#define YOMOTSU_MACRODEF_H_

#include <kern/types.h>

#ifdef _KERNEL

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
void __panic(const char *func, uint line, const char *fmt, ...) NORETURN;

        #define PANIC(fmt, ...) \
                __panic(__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

        #ifdef _KDEBUG
                #define ASSERT(x)                                          \
                        do {                                               \
                                if (!(x)) {                                \
                                        PANIC("assertion failed: %s", #x); \
                                }                                          \
                        } while (0)
        #else /* _KDEBUG */
                #define ASSERT(x) DONTCARE(x)
        #endif /* _KDEBUG */

        #define VERIFY(x, fmt, ...)                              \
                do {                                             \
                        if (!(x)) { PANIC(fmt, ##__VA_ARGS__); } \
                } while (0)

#endif /* _KERNEL */

#endif /* YOMOTSU_MACRODEF_H_ */
