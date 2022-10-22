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
#include <osrt/cdefs.h>

/* Integral type to store an error code */
typedef __osrt_uint __osrt_error_t;

/* Error code definitions */

#define __E(_err) __OSRT_PF_MNAME(ERROR_##_err)

enum __OSRT_ERROR_CODES {
        __OSRT_PF_MNAME(OK) = 0,
        __E(INVAL),
        __E(NOENT),
        __E(NOMEM),
        __E(DENIED),
        __E(TIMEDOUT),
	__E(EXIST),

        __E(PORT_REQ_CANCELED),
        __E(PORT_REQ_TOO_LONG),
        __E(PORT_NAME_OCCUPIED),
        __E(PORT_CLOSED),
	__E(END),
};

#undef __E

#ifdef __RZ_KERNEL

typedef __osrt_error_t error_t;

#define ERROR(_type) (ERROR_##_type)

#endif /* __RZ_KERNEL */

#endif /* __RENZAN_OSRT_ERROR_H__ */
