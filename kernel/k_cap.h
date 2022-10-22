/* k_cap.h -- Capability system implementaion */

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

#ifndef __RENZAN_K_CAP_H__
#define __RENZAN_K_CAP_H__

#include <k_atomic.h>
#include <k_list.h>
#include <k_spinlock.h>

#include <osrt/cap.h>
#include <osrt/error.h>
#include <osrt/types.h>

struct untyped_attrs {
        uintptr start;
        uintptr watermark;
        uintptr end;

        boolean device;
};

struct cap_s {
        spinlock_t  guard;
        cap_type_t  type;  /* Capability type */
        void *      ptr;   /* Pointer to the referenced object */
        cap_perms_t perms; /* Permissions */

        /* Capability dependency tree */
        struct cap_s *captree_parent;
        list_node_t   captree_siblings;
        list_node_t   captree_childs;

        union {
                struct untyped_attrs untyped;
        } attrs;
};

typedef struct cap_s cap_t;

void cap_retype(
    cap_t *dst, cap_t *src, cap_type_t new_type, cap_perms_t perms, size_t size,
    error_t *err);
void cap_mint(cap_t *dst, cap_t *src, cap_perms_t new_perms, error_t *err);
void cap_revoke_childs(cap_t *cap);
void cap_delete(cap_t *cap);

boolean cap_check_perm(cap_t *cap, cap_perm_entry_t perm);

#endif /* __RENZAN_K_CAP_H__ */
