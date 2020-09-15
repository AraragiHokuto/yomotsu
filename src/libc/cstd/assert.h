#ifndef __ORIHIME_CSTD_ASSERT_H__
#define __ORIHIME_CSTD_ASSERT_H__

#define static_assert _Static_assert

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
void __assert(
    int val, const char *expr, const char *file, int line, const char *func);
#define assert(expr) __assert((expr), #expr, __FILE__, __LINE__, __func__);
#endif

#endif /* __ORIHIME_CSTD_ASSERT_H__ */
