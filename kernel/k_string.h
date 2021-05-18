/* k_string.h -- String manipulation utilities */
/* XXX should move to KRT */

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

#ifndef __RENZAN_K_STRING_H__
#define __RENZAN_K_STRING_H__

#include <osrt/types.h>

#define KSTRLEN_MAX_SIZE_UNKNOWN 1048576

size_t  kstrlen(size_t max_scan, const char *str);
boolean kstrequ(size_t max_scan, const char *a, const char *b);
size_t  kstrcpy(size_t max_copy, char *dst, const char *src);

/* DJBX33A hash */
size_t kstr_hash(const char *src, size_t strlen);

void kmemcpy(void *dst, const void *src, size_t size);
void kmemmov(void *dst, const void *src, size_t size);
void kmemset(void *dst, byte c, size_t size);

#endif /* __RENZAN_K_STRING_H__ */
