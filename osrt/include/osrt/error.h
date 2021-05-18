/* osrt/error.h -- Error code definitions */

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

#ifndef __RENZAN_OSRT_ERROR_H__
#define __RENZAN_OSRT_ERROR_H__

#include <osrt/types.h>

/* Integral type to store an error code */
typedef __osrt_uint __osrt_error_t;

/* Error code definitions */

enum __OSRT_ERROR_CODES {
        __OSRT_OK = 0,
        __OSRT_ERROR_INVAL,
        __OSRT_ERROR_NOENT,
        __OSRT_ERROR_NOMEM,
        __OSRT_ERROR_DENIED,
        __OSRT_ERROR_TIMEDOUT,

        __OSRT_ERROR_PORT_REQ_CANCELED,
        __OSRT_ERROR_PORT_REQ_TOO_LONG,
        __OSRT_ERROR_PORT_NAME_OCCUPIED,
        __OSRT_ERROR_PORT_CLOSED,
        __OSRT_ERROR_END
};

#ifdef __RZ_KERNEL

typedef __osrt_error_t error_t;

#define ERROR(_type) (__OSRT_ERROR_##_type)
#define OK           __OSRT_OK

#endif /* __RZ_KERNEL */

#endif /* __RENZAN_OSRT_ERROR_H__ */
