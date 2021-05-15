/* k_cap.h -- Kernel capability implementations */

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

#ifndef __RENZAN_K_CAP_H__
#define __RENZAN_K_CAP_H__

#include <os/kernel/atomic.h>
#include <osrt/capability.h>
#include <osrt/error.h>
#include <osrt/types.h>

typedef struct cap_info cap_info_t;

/* Object destructor callback type */
typedef void (*cap_dtor_cb_t)(cap_info_t *);

/*
 * Capability info structure for referenced object
 * All structures referenced by a capability must embed this structure
 */
struct cap_info {
        uint          type; /* referenced object type. read only after ctor */
        atomic_uint   rc;   /* reference count */
        cap_dtor_cb_t dtor;
};

/* In-kernel protected capability structure */
typedef struct cap {
        cap_info_t *       ptr;
        cap_right_bitmap_t rights;
} cap_t;

/* Get userland info for a capability */
error_t cap_get_userland(
    cap_userland_t *dst, cap_selector_t selector, const cap_t *src);
/* Duplicate a capability */
error_t cap_dup(cap_t *dst, const cap_t *src);
/* Set only given rights on a capability */
error_t cap_set_rights(cap_t *cap, cap_right_bitmap_t rights);
/* Unset given rights on a capability */
error_t cap_unset_rights(cap_t *cap, cap_right_bitmap_t rights);
/* Delete a capability */
void cap_del(cap_t *cap);

#endif /* __RENZAN_K_CAP_H__ */
