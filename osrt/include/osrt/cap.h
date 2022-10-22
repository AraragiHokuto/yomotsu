/* cap.h -- Capability definitions */

/*
 * Copyright 2022 Tenhouin Youkou <youkou@tenhou.in>.
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

#ifndef __RENZAN_OSRT_CAP_H__
#define __RENZAN_OSRT_CAP_H__

#include <osrt/cdefs.h>
#include <osrt/types.h>

#define __OSRT_CAP_TYPE(type) __OSRT_PF_MNAME(CAP_TYPE_##type)

/* Capability types */
enum __OSRT_CAP_TYPES {
	__OSRT_CAP_TYPE(NULL) = 0,
	__OSRT_CAP_TYPE(UNTYPED), /* Untyped physical memory */
	__OSRT_CAP_TYPE(FRAME),	/* Page frame */
	__OSRT_CAP_TYPE(ENDPOINT), /* IPC Endpoint */
	__OSRT_CAP_TYPE(CNODE),	   /* Container of capabilities */
	__OSRT_CAP_TYPE(COUNT)
};

/* Capability permission list */
typedef __osrt_u64 __OSRT_PF_TNAME(cap_perms_t);
typedef __osrt_u64 __OSRT_PF_TNAME(cap_type_t);

/* Capability permission entry */
typedef __osrt_uint __OSRT_PF_TNAME(cap_perm_entry_t);

#endif /* __RENZAN_OSRT_CAP_H__ */
