/* string.h -- String handling */

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

#ifndef __RENZAN_CSTD_STRING_H_
#define __RENZAN_CSTD_STRING_H_

#define __need_size_t
#include <stddef.h>

/* Copying functions */
void *memcpy(void *restrict s1, const void *restrict s2, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
char *strcpy(char *restrict s1, const char *restrict s2);
char *strncpy(char *restrict s1, const char *restrict s2, size_t n);

/* Concatenation functions */
char *strcat(char *restrict s1, const char *restrict s2);
char *strncat(char *restrict s1, const char *restrict s2, size_t n);

/* Comparison functions */
int    memcmp(const void *s1, const void *s2, size_t n);
int    strcmp(const char *s1, const char *s2);
int    strcoll(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n);

/* Search functions */
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
size_t strcspn(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char *restrict s1, const char *restrict s2);

/* Miscellaneous functions */
void *memset(void *s, int c, size_t n);
char *strerror(int errnum);
size_t strlen(const char *s);

#endif /* __RENZAN_CSTD_STRING_H_ */
