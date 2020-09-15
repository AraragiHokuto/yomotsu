#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

/* Copy n bytes from src to dst, assume no overlap */
static void
__memcpy_rep_movsb(void *restrict dst, const void *restrict src, size_t n)
{
        asm volatile("rep movsb" ::"D"(dst), "S"(src), "c"(n) : "memory");
}

/* fill n bytes starting from dst with byte `c` */
static void
__memset_rep_stosb(void *dst, unsigned char c, size_t n)
{
        asm volatile("rep stosb" ::"D"(dst), "a"(c), "c"(n) : "memory");
}

/* compare n bytes from dst with src, return index of first different byte */
static size_t
__memcmp_repne_cmpsb(const void *dst, const void *src, size_t n)
{
        size_t scanned;
        asm volatile("repne movsb"
                     : "=c"(scanned)
                     : "D"(dst), "S"(src), "c"(n));
        return n - scanned;
}

/* find byte 'c' in first n bytes of s, return bytes scanned */
static size_t
__memchr_repne_scasb(const void *s, unsigned char c, size_t n)
{
        size_t scanned;
        asm volatile("repne scasb" : "=c"(scanned) : "D"(s), "a"(c), "c"(n));
        return n - scanned;
}

#define __memcpy_impl __memcpy_rep_movsb
#define __memset_impl __memset_rep_stosb
#define __memcmp_impl __memcmp_repne_cmpsb
#define __memchr_impl __memchr_repne_scasb

void *
memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
        __memcpy_impl(s1, s2, n);
        return s1;
}

void *
memmove(void *s1, const void *s2, size_t n)
{
        if (s1 == s2) {
                /* do nothing */
        } else if (
            (uintptr_t)s1 < (uintptr_t)s2
            && (uintptr_t)s1 + n <= (uintptr_t)s2) {
                /* no overlap */
                __memcpy_impl(s1, s2, n);
        } else if (
            (uintptr_t)s2 < (uintptr_t)s1
            && (uintptr_t)s2 + n <= (uintptr_t)s1) {
                /* no overlap */
                __memcpy_impl(s1, s2, n);
        } else if ((uintptr_t)s1 < (uintptr_t)s2) {
                /* s1 in front */
                size_t overlap = (s1 + n) - s2;

                /* move memory in `overlap` size chunks */
                for (size_t moved = 0; moved < n; moved += overlap) {
                        size_t remaining = n - moved;
                        size_t chunk_size =
                            remaining < overlap ? remaining : overlap;
                        unsigned char *_p1 = (unsigned char *)s1 + moved;
                        unsigned char *_p2 = (unsigned char *)s2 + moved;
                        __memcpy_impl(_p1, _p2, chunk_size);
                }
        } else if ((uintptr_t)s2 < (uintptr_t)s1) {
                /* s2 in front */
                size_t overlap = (s2 + n) - s1;

                /* move memory in overlap_size chunks, in reverse order */
                for (size_t moved = 0; moved < n; moved += overlap) {
                        size_t remaining = n - moved;
                        size_t chunk_size =
                            remaining < overlap ? remaining : overlap;
                        unsigned char *_p1 = (unsigned char *)s1 + n - moved;
                        unsigned char *_p2 = (unsigned char *)s2 + n - moved;
                        __memcpy_impl(
                            _p1 - chunk_size, _p2 - chunk_size, chunk_size);
                }
        }
        return s1;
}

char *
strcpy(char *restrict s1, const char *restrict s2)
{
        return strncpy(s1, s2, strlen(s2));
}

char *
strncpy(char *restrict s1, const char *restrict s2, size_t n)
{
        size_t s2_len   = __memchr_impl(s2, '\0', n);
        size_t fill_len = n - s2_len;

        __memcpy_impl(s1, s2, s2_len);
        if (fill_len) __memset_impl(s1 + s2_len, '\0', fill_len);
        return s1;
}

char *
strcat(char *restrict s1, const char *restrict s2)
{
        return strncat(s1, s2, __SIZE_MAX__);
}

char *
strncat(char *restrict s1, const char *restrict s2, size_t n)
{
        size_t s1_len = __memchr_impl(s1, '\0', __SIZE_MAX__);
        size_t s2_len = __memchr_impl(s2, '\0', n);
        __memcpy_impl(s1 + s1_len, s2, s2_len);
        s1[s1_len + s2_len] = '\0';
        return s1;
}

int
memcmp(const void *s1, const void *s2, size_t n)
{
        size_t diff_idx = __memcmp_impl(s1, s2, n);

        if (n == diff_idx) { return 0; }

        unsigned char c1 = diff_idx[(const unsigned char *)s1];
        unsigned char c2 = diff_idx[(const unsigned char *)s2];
        if (c1 == c2) { return 0; }

        return c1 > c2 ? 1 : -1;
}

int
strcmp(const char *s1, const char *s2)
{
        return strncmp(s1, s2, __SIZE_MAX__);
}

/* TODO: locale */
int
strcoll(const char *s1, const char *s2)
{
        return strcmp(s1, s2);
}

int
strncmp(const char *s1, const char *s2, size_t n)
{
        size_t s1_cmp_size = __memchr_impl(s1, '\0', n);
        size_t s2_cmp_size = __memchr_impl(s2, '\0', n);

        size_t cmp_size = s1_cmp_size < s2_cmp_size ? s1_cmp_size : s2_cmp_size;
        return memcmp(s1, s2, cmp_size + 1); /* including NULL terminator */
}

/* TODO: locale */
size_t
strxfrm(char *restrict s1, const char *restrict s2, size_t n)
{
        if (n == 0 && !s1) {
                /* do nothing */
        } else {
                strncpy(s1, s2, n);
        }

        return strlen(s2);
}

void *
memchr(const void *s, int c, size_t n)
{
        size_t idx = __memchr_impl(s, c, n);
        return idx == n ? NULL : (unsigned char *)s + idx;
}

char *
strchr(const char *s, int c)
{
        return memchr(s, c, strlen(s) + 1);
}

size_t
strcspn(const char *s1, const char *s2)
{
        bool   s2_map[UCHAR_MAX + 1] = {0};
        size_t s1_len                = strlen(s1);
        size_t s2_len                = strlen(s2);

        for (size_t i = 0; i < s2_len; ++i) {
                s2_map[(unsigned char)s2[i]] = true;
        }

        size_t ret;
        for (ret = 0; ret < s1_len; ++ret) {
                if (s2_map[(unsigned char)s1[ret]]) { break; }
        }

        return ret;
}

char *
strpbrk(const char *s1, const char *s2)
{
        bool   s2_map[UCHAR_MAX + 1] = {0};
        size_t s1_len                = strlen(s1);
        size_t s2_len                = strlen(s2);

        for (size_t i = 0; i < s2_len; ++i) {
                s2_map[(unsigned char)s2[i]] = true;
        }

        for (size_t i = 0; i < s1_len; ++i) {
                if (s2_map[(unsigned char)s1[i]]) { return (char *)s1 + i; }
        }

        return NULL;
}

/* TODO: better implementation? */
char *
strrchr(const char *s, int _c)
{
        unsigned char c = _c;
        for (size_t i = strlen(s); i > 0; --i) {
                if (s[i] == c) { return (char *)s + i; }
        }

        return s[0] == c ? (char *)s : NULL;
}

size_t
strspn(const char *s1, const char *s2)
{
        bool   s2_map[UCHAR_MAX + 1] = {0};
        size_t s1_len                = strlen(s1);
        size_t s2_len                = strlen(s2);

        for (size_t i = 0; i < s2_len; ++i) {
                s2_map[(unsigned char)s2[i]] = true;
        }

        size_t ret;
        for (ret = 0; ret < s1_len; ++ret) {
                if (!s2_map[(unsigned char)s1[ret]]) { break; }
        }

        return ret;
}

/* TODO: KMP? */
char *
strstr(const char *s1, const char *s2)
{
        size_t s1_len = strlen(s1);
        size_t s2_len = strlen(s2);

        if (!s2_len) { return (char *)s1; }

        if (s2_len > s1_len) { return NULL; }

        for (size_t i = 0; i <= s1_len - s2_len; ++i) {
                if (strncmp(&s1[i], s2, s2_len) == 0) { return (char *)s1 + i; }
        }

        return NULL;
}

/* TODO: thread_local */
static char *__strtok_saved_ptr = NULL;

char *
strtok(char *restrict s1, const char *restrict s2)
{
        bool s2_map[UCHAR_MAX + 1] = {0};

        size_t s2_len = strlen(s2);

        /* build s2_map */
        for (size_t i = 0; i < s2_len; ++i) {
                s2_map[(unsigned char)s2[i]] = true;
        }

        char *s = s1 ? s1 : __strtok_saved_ptr;
        if (!s) { return NULL; }
        size_t s_len = strlen(s);

        char *ret = NULL;
        for (size_t i = 0; i < s_len; ++i) {
                if (!s2_map[(unsigned char)s[i]]) {
                        s_len -= i;
                        ret = &s[i];
                        break;
                }
        }

        if (!ret) { return ret; }

        for (size_t i = 0; i < s_len; ++i) {
                if (s2_map[(unsigned char)ret[i]]) {
                        ret[i]             = '\0';
                        __strtok_saved_ptr = &ret[i + 1];
                        return ret;
                }
        }

        __strtok_saved_ptr = NULL;
        return ret;
}

void *
memset(void *s, int c, size_t n)
{
        __memset_impl(s, c, n);
        return s;
}

char *
strerror(int errnum)
{
	switch (errnum) {
	case _OK:
		return "OK";
	case ENOMEM:
		return "Out of memory";
	case EPERM:
		return "Forbidden";
	case EINVAL:
		return "Invalid argument";
	case ENOENT:
		return "No entity";
	case ETIMEDOUT:
		return "Timed out";
	case EDOM:
		return "EDOM";
	case EILSEQ:
		return "EILSEQ";
	case ERANGE:
		return "Range error;";
	case EOVERFLOW:
		return "Overflow";
	default:
		return "Unknown error";
	}
}

size_t
strlen(const char *s)
{
        return __memchr_impl(s, '\0', __SIZE_MAX__);
}
