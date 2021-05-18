/* errno.h -- Errors */

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

#ifndef __RENZAN_CSTD_ERRNO_H__
#define __RENZAN_CSTD_ERRNO_H__

#include <osrt/error.h>

#define __ERROR_OFFSET(no) (__OSRT_ERROR_END + no)

/* kernel -> POSIX error code mapping */
#define _OK    __OSRT_OK
#define EINVAL __OSRT_ERROR_INVAL
#define ENOENT __OSRT_ERROR_NOENT
#define ENOMEM __OSRT_ERROR_NOMEM
#define EPERM  __OSRT_ERROR_DENIED

/* stdc error code */
#define EDOM      __ERROR_OFFSET(1)
#define EILSEQ    __ERROR_OFFSET(2)
#define ERANGE    __ERROR_OFFSET(3)
#define EOVERFLOW __ERROR_OFFSET(4)

/* TODO: generate POSIX error codes */

int *__get_errno(void);

#define errno (*__get_errno())

#endif /* __RENZAN_CSTD_ERRNO_H__ */
