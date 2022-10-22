/* k_cap.c -- Capability system */

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

#include <k_cap.h>
#include <k_cdefs.h>
#include <k_cnode.h>
#include <k_untyped.h>

static boolean
is_power_of_2(size_t size)
{
        return size != 0 && (size & (size - 1)) == 0;
}

void
cap_retype(
    cap_t *dst, cap_t *src, cap_type_t new_type, cap_perms_t perms, size_t size,
    error_t *_err)
{
        ASSERT(new_type < CAP_TYPE_COUNT);
        ASSERT(dst->type == CAP_TYPE_NULL);
        ASSERT(src->type == CAP_TYPE_UNTYPED);

        error_t err = 0;

        size_t mem_size = 0;
        size_t align    = 1;

        switch (new_type) {
        case CAP_TYPE_UNTYPED:
                mem_size = size;
                break;
        case CAP_TYPE_FRAME:
                /* TODO: frame size */
                break;
        case CAP_TYPE_ENDPOINT:
                /* TODO: endpoint size */
                break;
        case CAP_TYPE_CNODE:
                if (!is_power_of_2(size)) {
                        /* CNode size must be power of 2 */
                        err = ERROR_INVAL;
                        return;
                }
                mem_size = sizeof(cnode_t) + sizeof(cap_t) * size;
                align    = __alignof__(cap_t);
                break;
        default:
                UNREACHABLE;
        }

        void *ptr = untyped_alloc(src->ptr, mem_size, align, &err);
        if (err) {
                *_err = err;
                return;
        }

        /* Set up basic infos for the new cap */
        dst->type  = new_type;
        dst->perms = perms;
        dst->ptr   = ptr;

        /* Insert the new cap into captree */
        dst->captree_parent = src;
        list_head_init(&dst->captree_childs);
        list_insert(&dst->captree_siblings, src->captree_childs.next);

        /* Type-related init operations */
        switch (new_type) {
        case CAP_TYPE_UNTYPED:
                untyped_init(
                    dst, (uintptr)ptr, size, src->attrs.untyped.device);
                break;
        case CAP_TYPE_FRAME:
                /* TODO */
                break;
        case CAP_TYPE_ENDPOINT:
                /* TODO */
                break;
        case CAP_TYPE_CNODE:
                cnode_init(dst, size);
                break;
        default:
                UNREACHABLE;
        }
}

void
cap_mint(cap_t *dst, cap_t *src, cap_perms_t new_perms, error_t *err)
{
	ASSERT(dst->type == CAP_TYPE_NULL);
	ASSERT(src->type != CAP_TYPE_NULL);

        /* Type-related mint operations */
        switch (src->type) {
        case CAP_TYPE_UNTYPED:
                untyped_mint(dst, src, err);
                break;
        case CAP_TYPE_FRAME:
                /* TODO */
                break;
        case CAP_TYPE_ENDPOINT:
                /* TODO */
                break;
        case CAP_TYPE_CNODE:
                cnode_mint(dst, src, err);
                break;
        default:
                UNREACHABLE;
        }

        /* Set type of dst */
        dst->type = src->type;

        /* Set up new permissions */
        dst->perms = new_perms & src->perms;

        /* Insert dst as src's child */
        dst->captree_parent = src;
        list_insert(&dst->captree_siblings, &src->captree_childs);
}

/*
 * Delete a capability
 * cap must have all its childs deleted beforehand
 * Must hold lock to cap and cap's parent
 */
static void
do_cap_delete(cap_t *cap)
{
        ASSERT(list_is_empty(&cap->captree_childs));

        if (cap->type == CAP_TYPE_NULL) {
                /* NULL cap; do nothing */
                return;
        }

        /* Type-related delete operations */
        switch (cap->type) {
        case CAP_TYPE_UNTYPED:
                untyped_delete(cap);
                break;
        case CAP_TYPE_FRAME:
                /* TODO */
                break;
        case CAP_TYPE_ENDPOINT:
                /* TODO */
                break;
        case CAP_TYPE_CNODE:
                cnode_delete(cap);
                break;
        default:
                UNREACHABLE;
        }

        /* Remove cap from captree */
        list_remove(&cap->captree_siblings);
        cap->captree_parent = NULL;

        /* Reset cap to null */
        cap->type = CAP_TYPE_NULL;
}

/*
 * Delete all childs of a capability
 * Must hold lock to cap
 */
static void
do_cap_revoke_childs(cap_t *cap)
{
        if (list_is_empty(&cap->captree_childs)) {
                /* No child; We're good */
                return;
        }

        /* Remove all childs */
        LIST_FOREACH_MUT(cap->captree_childs, p, np)
        {
                cap_t *c = CONTAINER_OF(p, cap_t, captree_siblings);

                spinlock_lock(&c->guard);
                do_cap_revoke_childs(c);
                do_cap_delete(c);
                spinlock_unlock(&c->guard);
        }
}

void
cap_revoke_childs(cap_t *cap)
{
        spinlock_lock(&cap->guard);
        do_cap_revoke_childs(cap);
        spinlock_unlock(&cap->guard);
}

void
cap_delete(cap_t *cap)
{
        /* Revoke all childs of cap */
        do_cap_revoke_childs(cap);

        cap_t *parent = cap->captree_parent;
        if (parent) {
                /* Attemp to lock cap's parent first.
                 * If failed, rollback and lock from parent to cap
                 * in order to prevent deadlock. */
                if (!spinlock_try_lock(&parent->guard)) {
                        spinlock_unlock(&cap->guard);
                        spinlock_lock(&parent->guard);
                        spinlock_lock(&cap->guard);
                }

                /* At this point we should hold lock for both parent and cap.
                 * Proceed. */
        }

        do_cap_delete(cap);

        if (parent) { spinlock_unlock(&parent->guard); }
}

boolean
cap_check_perm(cap_t *cap, cap_perm_entry_t perm)
{
	return !!(cap->perms & (1 << perm));
}
