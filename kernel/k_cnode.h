/* k_cnode.h -- CNode and CNode capabilities */

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

#ifndef __RENZAN_K_CNODE_H__
#define __RENZAN_K_CNODE_H__

#include <k_atomic.h>
#include <k_cap.h>

#include <osrt/cnode.h>

struct cnode_s {
        atomic_uint ref_count; /* reference count */
        /* Order of the size of cnode
         * e.g. size == 256 == 2^8, then order == 8 */
        size_t order;
        cap_t  caps[];
};

typedef struct cnode_s cnode_t;

void   cnode_init(cap_t *cnode, size_t cnode_size);
void   cnode_mint(cap_t *dst, cap_t *src, error_t *err);
void   cnode_delete(cap_t *cap);
cap_t *cnode_lookup_and_lock(
    cap_t *root, cnode_addr_t addr, cap_t **container, error_t *err);

#endif /* __RENZAN_K_CNODE_H__ */
