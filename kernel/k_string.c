#include <k_cdefs.h>
#include <k_string.h>

/*
 * Scan a string pointed to by str, return number of chars
 * scanned when a '\0' is encountered or max_scan is reached.
 * if max_scan is reached before '\0', return max_scan.
 * '\0' is not counted in return value.
 */
size_t
kstrlen(size_t max_scan, const char *str)
{
        size_t ret = 0;
        for (; ret < max_scan; ++ret) {
                if (str[ret] == '\0') { break; }
        }
        return ret;
}

/*
 * Compare string pointed to by a and b, return B_TRUE if
 * both string have same length, and every char in both
 * string compares equal.
 */
boolean
kstrequ(size_t max_scan, const char *a, const char *b)
{
        size_t sa = kstrlen(max_scan, a);
        size_t sb = kstrlen(max_scan, b);

        if (sa != sb) { return B_FALSE; }

        for (size_t i = 0; i < sa; ++i) {
                if (a[i] != b[i]) { return B_FALSE; }
        }

        return B_TRUE;
}

/*
 * Copy a NULL-terminated string from src to dst, return kstrlen(max_copy, src).
 * If src is longer than max_copy (i.e. kstrlen(max_copy, src) == max_copy),
 * (size_t)-1 is return; no coping is done in that case.
 */
size_t
kstrcpy(size_t max_copy, char *dst, const char *src)
{
        size_t src_len = kstrlen(max_copy, src);
        if (src_len == max_copy) { return (size_t)-1; }

        kmemcpy(dst, src, src_len);
        dst[src_len + 1] = '\0';
        return src_len;
}

/*
 * Calculate the DJBX33A hash value of str
 */
size_t
kstr_hash(const char *str, size_t strlen)
{
        size_t ret = 0;
        for (size_t i = 0; i < strlen; ++i) { ret = (ret << 5) + ret + str[i]; }
        return ret;
}

static void
__do_memcpy(void *dst, const void *src, size_t size)
{
        /* TODO?: Better implementation for older CPU */
        asm volatile("rep movsb"
                     : "=D"(dst), "=S"(src), "=c"(size)
                     : "0"(dst), "1"(src), "2"(size)
                     : "memory");
}

/*
 * Copy size bytes from src to memory area pointed to by dst.
 * if memory area pointed to by src and dst overlaps, the
 * behavior is undefined.
 */
void
kmemcpy(void *dst, const void *src, size_t size)
{
        uintptr _dst = (uintptr)dst;
        uintptr _src = (uintptr)src;

        /* Ensure memory does not overlap */
        ASSERT(!((_src >= _dst) && (_src < _dst + size)));
        ASSERT(!((_dst >= _src) && (_dst < _src + size)));

        __do_memcpy(dst, src, size);
}

/*
 * Copy size bytes from src to memory area pointed to by dst.
 * This routine will correctly handle the case in which two
 * memory region overlaps
 */
void
kmemmov(void *dst, const void *src, size_t size)
{
        /* if dst overlaps with src */
        byte *_dst = (byte *)dst;
        byte *_src = (byte *)src;

        byte *_src_end = _src + size;

        if (_dst >= _src && _dst <= _src_end) {
                byte *o_begin = _dst;
                byte *o_end   = _src_end;
                byte *o_dst   = _dst + (_dst - _src);

                __do_memcpy(o_dst, o_begin, o_end - o_begin);

                _src_end = _dst;
        }

        __do_memcpy(_dst, _src, _src_end - _src);
}

/*
 * Fill size bytes of memory pointed to by dst with c.
 * Note that this routine has a different signature
 * than memset() in libc.
 */
void
kmemset(void *dst, byte c, size_t size)
{
        /* TODO?: Better implementation for older CPU */
        asm volatile("rep stosb"
                     : "=D"(dst), "=a"(c), "=c"(size)
                     : "0"(dst), "1"(c), "2"(size)
                     : "memory");
}
