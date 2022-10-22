/* k_cnode.c -- CNode and CNode capabilities */

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

#include <k_cdefs.h>
#include <k_cnode.h>
#include <k_string.h>

#include <osrt/types.h>

/* size must be a power of 2 */
size_t
order_to_size(size_t size)
{
        size_t ret;
        for (ret = 0; size != 1; ++ret) { size >>= 1; }

        return ret;
}

size_t
size_to_order(size_t order)
{
        return 1 << order;
}

cnode_addr_t
mask_from_order(size_t order)
{
        cnode_addr_t ret = 1;
        ret <<= order;
        ret -= 1;
        return ret;
}

static void
cnode_inc_ref(cnode_t *cnode)
{
        atomic_inc_fetch_uint(cnode->ref_count, __ATOMIC_RELAXED);
}

static void
cnode_dec_ref(cnode_t *cnode)
{
        uint ret = atomic_dec_fetch_uint(cnode->ref_count, __ATOMIC_ACQ_REL);
        if (ret == 0) {
                /* Destroy all caps in the cnode */
                for (size_t i = 0; i < order_to_size(cnode->order); ++i) {
                        cap_delete(&cnode->caps[i]);
                }
        }
}

void
cnode_init(cap_t *cap, size_t cnode_size)
{
        cnode_t *cnode = cap->ptr;

        kmemset(cnode, 0, sizeof(cnode_t) + sizeof(cap_t) * cnode_size);

        cnode->order = size_to_order(cnode_size);
        cnode_inc_ref(cnode);
}

void
cnode_mint(cap_t *dst, cap_t *src, error_t *err)
{
        cnode_inc_ref(src->ptr);
        dst->ptr = src->ptr;
}

void
cnode_delete(cap_t *cap)
{
        cnode_dec_ref(cap->ptr);
}

/* XXX move this to arch specific headers */
#define CNODEADDR_INDEXES_BITS 56

static size_t
cnode_addr_get_depth(cnode_addr_t addr)
{
        return addr >> CNODEADDR_INDEXES_BITS;
}

#define ADDR_MASK(_order) (((size_t)1 << (_order)) - 1);

static size_t
cnode_addr_get_indexes(cnode_addr_t addr)
{
        return addr & ADDR_MASK(CNODEADDR_INDEXES_BITS);
}

cap_t *
cnode_lookup_and_lock(cap_t *root, cnode_addr_t addr, error_t *err)
{
        cap_t *p = root;

        size_t depth   = cnode_addr_get_depth(addr);
        size_t indexes = cnode_addr_get_indexes(addr);

        for (size_t i = 0; i < depth; ++i) {
                if (p->type != CAP_TYPE_CNODE) {
                        spinlock_unlock(&p->guard);
                        *err = ERROR_INVAL;
                        return NULL;
                }

                cnode_t *cnode = p->ptr;

                size_t idx = indexes & ADDR_MASK(cnode->order);
                indexes >>= cnode->order;

                cap_t *np = &cnode->caps[idx];

                spinlock_lock(&np->guard);
                spinlock_unlock(&p->guard);

                p = np;
        }

        if (indexes != 0) {
                spinlock_unlock(&p->guard);
                *err = ERROR_INVAL;
                return NULL;
        }

        return p;
}
