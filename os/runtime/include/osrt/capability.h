/* osrt/capability.h -- capability related definitions in OSRT */

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

#ifndef __RENZAN_OSRT_CAPABILITY_H__
#define __RENZAN_OSRT_CAPABILITY_H__

#include <osrt/types.h>

/* Selector into process's cap table */
typedef __osrt_uint __osrt_cap_selector_t;

/* Capability access right bitmap */
typedef __osrt_u64 __osrt_cap_right_bitmap_t;

#define __OSRT_CAP_TYPE_LIMIT 64

enum __OSRT_CAP_TYPE {
        __OSRT_CAP_TYPE_RESV = 0,
        __OSRT_CAP_TYPE_PROCESS,
        __OSRT_CAP_TYPE_MEM_REG,
        __OSRT_CAP_TYPE_ENDPOINT,
        __OSRT_CAP_TYPE_MAX
};

/* Userland capability info structure */
typedef struct __osrt_cap {
        __osrt_uint               type;
        __osrt_cap_selector_t     selector;
        __osrt_cap_right_bitmap_t rights;
} __osrt_cap_t;

/* Capability operation flags */
enum __OSRT_CAP_OPF {
        __OSRT_CAP_OPF_RESV = 0,
        __OSRT_CAP_OPF_GET  = 1,  /* get capability info by selector */
        __OSRT_CAP_OPF_DUP  = 2,  /* duplicate capability. */
        __OSRT_CAP_OPF_SET  = 4,  /* set only given access rights on a cap.
                                   * can be used together with DUP. */
        __OSRT_CAP_OPF_UNSET = 8, /* unset given access rights on a cap.
                                   * must be used with __OSRT_CAP_OPF_SET. */
        __OSRT_CAP_OPF_DEL = 16,  /* delete a capability. */
};

#ifdef __RZ_KERNEL

/* Kernel space definitions */
typedef __osrt_cap_selector_t     cap_selector_t;
typedef __osrt_cap_right_bitmap_t cap_right_bitmap_t;
typedef __osrt_cap_t              cap_userland_t;

#endif /* __RZ_KERNEL */

#endif /* __RENZAN_OSRT_CAPABILITY_H__ */
