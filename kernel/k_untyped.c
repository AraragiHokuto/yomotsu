/* k_untyped.c -- Untyped capabilities */

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
#include <k_untyped.h>
#include <k_string.h>

#define ALIGN(addr, align) ((((addr - 1) / (align)) + 1) * (align))

void *
untyped_alloc(cap_t *untyped, size_t size, size_t align, error_t *err)
{
	ASSERT(untyped->type == CAP_TYPE_UNTYPED);

        uintptr ret = untyped->attrs.untyped.watermark;
        if (list_is_empty(&untyped->captree_childs)) {
                /* Fresh untyped object, allocate from start */
                ret = untyped->attrs.untyped.start;
        }

	/* Allocate memory */
        ret = ALIGN(ret, align);
        ret += size;

        if (ret >= untyped->attrs.untyped.end) {
                *err = ERROR_NOMEM;
                return NULL;
        }

	/* Update watermark */
	untyped->attrs.untyped.watermark = ret;

        return (void *)ret;
}

/*
 * Only called on retype operations; initial untyped objects
 * are created by the ignitor.
 */
void
untyped_init(cap_t *untyped, uintptr start, size_t size, boolean device)
{
	untyped->attrs.untyped.start = start;
	untyped->attrs.untyped.end = start + size;
	untyped->attrs.untyped.device = device;
}

/*
 * Untyped capabilities can not be minted
 */
void
untyped_mint(cap_t *_dst, cap_t *_src, error_t *err)
{
	DONTCARE(_dst);
	DONTCARE(_src);

	*err = ERROR_INVAL;
}

void
untyped_delete(cap_t *_untyped)
{
	DONTCARE(_untyped);
}
