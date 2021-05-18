/* signal.h -- Signal handling */

/*
 * Copyright 2021 Mosakuji Hokuto <shikieiki@yamaxanadu.org>.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __RENZAN_CSTD_SIGNAL_H__
#define __RENZAN_CSTD_SIGNAL_H__

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

#ifdef __RZ_OSRT
void __signal_init(void);
#endif /* __RZ_OSRT */

#endif /* __RENZAN_CSTD_SIGNAL_H__ */
