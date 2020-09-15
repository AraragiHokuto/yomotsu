/* stdio.c -- Input/output */
#include <errno.h>
#include <libos/console.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static size_t
_console_write_impl(FILE *f, const unsigned char *data, size_t size)
{
        (void)f;
        console_write((const char *)data, size);
        return size;
}

FILE __stderr = {
    .read  = NULL,
    .write = _console_write_impl,
    .seek  = NULL,

    .buf      = NULL,
    .buf_size = 0,

    .data = NULL};

FILE __stdout = {
    .read  = NULL,
    .write = _console_write_impl,
    .seek  = NULL,

    .buf      = NULL,
    .buf_size = 0,

    .data = NULL};

/** Formatted input/output functions **/
int
fprintf(FILE *restrict stream, const char *restrict format, ...)
{
        va_list ap;
        va_start(ap, format);
        int ret = vfprintf(stream, format, ap);
        va_end(ap);
        return ret;
}

int
printf(const char *restrict format, ...)
{
        va_list ap;
        va_start(ap, format);
        int ret = vprintf(format, ap);
        va_end(ap);
        return ret;
}

int
snprintf(char *restrict s, size_t n, const char *restrict format, ...)
{
        va_list ap;
        va_start(ap, format);
        int ret = vsnprintf(s, n, format, ap);
        va_end(ap);
        return ret;
}

int
sprintf(char *restrict s, const char *restrict format, ...)
{
        va_list ap;
        va_start(ap, format);
        int ret = vsprintf(s, format, ap);
        va_end(ap);
        return ret;
}

int
vprintf(const char *restrict format, va_list arg)
{
        return vfprintf(stdout, format, arg);
}

struct _sn_write_data_s {
        char * p;
        size_t n;
};

static size_t
_sn_write_impl(FILE *f, const unsigned char *data, size_t size)
{
        struct _sn_write_data_s *_f_data = f->data;
        size_t write_size = size < _f_data->n ? size : _f_data->n;

        if (_f_data->p && _f_data->n) {
                memcpy(_f_data->p, data, write_size);
                _f_data->p += write_size;
                _f_data->n -= write_size;
        }

        return size;
}

int
vsnprintf(char *restrict s, size_t n, const char *restrict format, va_list arg)
{
        struct _sn_write_data_s _f_data = {
            .p = s,
            .n = n,
        };

        FILE _f = {
            .read     = NULL,
            .write    = _sn_write_impl,
            .seek     = NULL,
            .data     = &_f_data,
            .buf      = NULL,
            .buf_size = 0,
        };

        return vfprintf(&_f, format, arg);
}

int
vsprintf(char *restrict s, const char *restrict format, va_list arg)
{
        return vsnprintf(s, __SIZE_MAX__, format, arg);
}

/** Error-handling functions **/
void
perror(const char *s)
{
        fprintf(stderr, "%s: %s\n", s, strerror(errno));
}
