/* osrt/types.h -- OS type definitions */

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

#ifndef __RENZAN_OSRT_TYPES_H__
#define __RENZAN_OSRT_TYPES_H__

#ifdef __RZ_KERNEL

/* Reserved identifiers to avoid userland namespace pollution */
typedef unsigned char  __osrt_u8;
typedef unsigned short __osrt_u16;
typedef unsigned int   __osrt_u32;
typedef unsigned long  __osrt_u64;

typedef __osrt_u64 __osrt_uint;

typedef signed char  __osrt_s8;
typedef signed short __osrt_s16;
typedef signed int   __osrt_s32;
typedef signed long  __osrt_s64;

typedef __osrt_s64 __osrt_sint;

typedef __osrt_uint __osrt_uintptr;
typedef __osrt_sint __osrt_sintptr;
typedef __osrt_u8   __osrt_byte;

/* Boolean value definitions */
typedef __osrt_u64 __osrt_boolean;

#define __OSRT_TRUE  ((__osrt_boolean)1)
#define __OSRT_FALSE ((__osrt_boolean)0)

/* Kernel space type definitions */
typedef __osrt_u8      u8;
typedef __osrt_u16     u16;
typedef __osrt_u32     u32;
typedef __osrt_u64     u64;
typedef __osrt_uint    uint;
typedef __osrt_s8      s8;
typedef __osrt_s16     s16;
typedef __osrt_s32     s32;
typedef __osrt_s64     s64;
typedef __osrt_sint    sint;
typedef __osrt_uintptr uintptr;
typedef __osrt_sintptr sintptr;
typedef __osrt_byte    byte;

/* Kernel space boolean definitions */
typedef __osrt_boolean boolean;

#else  /* __RZ_KERNEL */

#include <stdint.h>

/* Reserved identifiers to avoid userland namespace collision */

typedef uint8_t __osrt_u8;
typedef uint16_t __osrt_u16;
typedef uint32_t __osrt_u32;
typedef uint64_t __osrt_u64;

typedef uint64_t __osrt_uint;

typedef int8_t __osrt_s8;
typedef int16_t __osrt_s16;
typedef int32_t __osrt_s32;
typedef int64_t __osrt_s64;

typedef int64_t __osrt_sint;

typedef __osrt_uint __osrt_uintptr;
typedef __osrt_sint __osrt_sintptr;

typedef __osrt_u8 __osrt_byte;

typedef __osrt_u64 __osrt_boolean;

#define __OSRT_TRUE ((__osrt_boolean)1)
#define __OSRT_FALSE ((__osrt_boolean)0)

#endif	/* __RZ_KERNEL */

#endif /* __RENZAN_OSRT_TYPES_H__ */
