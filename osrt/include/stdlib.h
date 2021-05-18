/* stdlib.h -- General utilities */

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

#ifndef __RENZAN_CSTD_STDLIB_H__
#define __RENZAN_CSTD_STDLIB_H__

#define __need_NULL
#define __need_size_t
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifdef __INT_MAX__
#define RAND_MAX __INT_MAX__
#else
#define RAND_MAX 2147483647 /* same as INT_MAX */
#endif

/* Numeric conversion functions */
// double atof(const char *nptr);
int           atoi(const char *nptr);
long int      atol(const char *nptr);
long long int atoll(const char *nptr);

// double strtod(const char *restrict nptr, char ** restrict endptr);
// float strtof(const char *restrict nptr, char **restrict endptr);
// long double strtold(const char *restrict nptr, char **restrict endptr);
long int strtol(const char *restrict nptr, char **restrict endptr, int base);
long long int
strtoll(const char *restrict nptr, char **restrict endptr, int base);
unsigned long int
strtoul(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long int
strtoull(const char *restrict nptr, char **restrict endptr, int base);

/* Pseudo-random sequence generation functions */
int  rand(void);
void srand(unsigned int seed);

/* Memory management functions */
void *aligend_alloc(size_t alignment, size_t size);
void *calloc(size_t nmemb, size_t size);
void  free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

/* Communication with the environment */
void  abort(void) __attribute__((noreturn));
int   atexit(void (*func)(void));
int   at_quick_exit(void (*func)(void));
void  exit(int status) __attribute__((noreturn));
void  _Exit(int status) __attribute__((noreturn));
char *getenv(const char *name);
void  quick_exit(int status) __attribute__((noreturn));
int   system(const char *string);

/* Searching and sorting utilities */
// void *bsearch(
//    const void *key, const void *base, size_t nmemb, size_t size,
//    int (*compar)(const void *, const void *));
// void qsort(
//    void *base, size_t nmemb, size_t size,
//    int (*compar)(const void *, const void *));

/* Integer arithmetic functions */

int           abs(int j);
long int      labs(long int j);
long long int llabs(long long int j);

typedef struct __div_s {
        int quot;
        int rem;
} div_t;

typedef struct __ldiv_s {
        long quot;
        long rem;
} ldiv_t;

typedef struct __lldiv_s {
        long long quot;
        long long rem;
} lldiv_t;

div_t   div(int numer, int denom);
ldiv_t  ldiv(long int numer, long int denom);
lldiv_t lldiv(long long int numer, long long int denom);

/* Multibyte/wide character conversion functions */
/* TODO: want locale */

/* Multibyte/wide string conversion functions */
/* TODO: want locale */

#endif /* __RENZAN_CSTD_STDLIB_H__ */
