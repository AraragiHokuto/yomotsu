/* k_cap.c -- Kernel capability implementations */

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

#include <k_cap.h>
#include <os/kernel/cdefs.h>

error_t
cap_get_userland(cap_userland_t *dst, cap_selector_t selector, const cap_t *src)
{
	if (!src->ptr) {
		return ERROR(INVAL);
	}

	dst->selector = selector;
	dst->rights = src->rights;
	dst->type = src->ptr->type;

	return OK;
}

error_t
cap_dup(cap_t *dst, const cap_t *src)
{
	atomic_inc_fetch_uint(src->ptr->rc, __ATOMIC_RELAXED);
	dst->ptr = src->ptr;
	dst->rights = src->rights;

	return OK;
}

error_t
cap_set_rights(cap_t *cap, cap_right_bitmap_t rights)
{
	if (rights & (~cap->rights)) {
		/* Caps can only be converted into a less privileged version */
		return ERROR(DENIED);
	}

	cap->rights = rights;

	return OK;
}

error_t
cap_unset_rights(cap_t *cap, cap_right_bitmap_t rights)
{
	if (rights & (~cap->rights)) {
		/* Only rights that are held by cap can be unset */
		return ERROR(DENIED);
	}

	cap->rights &= (~rights);

	return OK;
}

void
cap_del(cap_t *cap)
{
	ASSERT(cap->ptr);
	cap_info_t *ref = cap->ptr;

	if (atomic_dec_fetch_uint(cap->ptr->rc, __ATOMIC_ACQ_REL) == 0) {
		/* Last out turns off the light */
		ref->dtor(ref);
	}
}
