/* k_untyped.h -- Untyped capabilities */

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

#ifndef __RENZAN_K_UNTYPED_H__
#define __RENZAN_K_UNTYPED_H__

#include <k_atomic.h>
#include <k_cap.h>
#include <k_spinlock.h>

#include <osrt/error.h>
#include <osrt/types.h>

void *untyped_alloc(cap_t *untyped, size_t size, size_t align, error_t *err);
void  untyped_init(cap_t *untyped, uintptr start, size_t size, boolean device);
void  untyped_mint(cap_t *dst, cap_t *src, error_t *err);
void  untyped_delete(cap_t *untyped);

#endif /* __RENZAN_K_UNTYPED_H__ */
