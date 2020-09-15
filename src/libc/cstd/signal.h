/* signal.h -- Signal handling */
#ifndef __ORIHIME_CSTD_SIGNAL_H__
#define __ORIHIME_CSTD_SIGNAL_H__

typedef volatile int sig_atomic_t;

enum { __SIG_DFL = 1, __SIG_ERR, __SIG_IGN };

#define SIG_DFL ((void *)__SIG_DFL)
#define SIG_ERR ((void *)__SIG_ERR)
#define SIG_IGN ((void *)__SIG_IGN)

enum {
        __SIGABRT,
        __SIGFPE,
        __SIGILL,
        __SIGINT,
        __SIGSEGV,
        __SIGTERM,
        __SIGNAL_MAX
};

#define SIGABRT __SIGABRT
#define SIGFPE  __SIGFPE
#define SIGILL  __SIGILL
#define SIGSEGV __SIGSEGV
#define SIGTERM __SIGTERM

/* Specify signal handling */
void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

#ifdef __ORIHIME_CSTD_INTERNAL
void __signal_init(void);
#endif /* __ORIHIME_CSTD_INTERNAL */

#endif /* __ORIHIME_CSTD_SIGNAL_H__ */
