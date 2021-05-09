/* stdio.h -- Input/Output */
#ifndef __RENZAN_CSTD_STDIO_H__
#define __RENZAN_CSTD_STDIO_H__

#define __need_NULL
#define __need_size_t
#include <stddef.h>

typedef struct __file_impl_s FILE;

#ifdef __RENZAN_OSRT

typedef unsigned long __off_t;

struct __file_impl_s {
        size_t (*read)(FILE *, unsigned char *, size_t);
        size_t (*write)(FILE *, const unsigned char *, size_t);
        __off_t (*seek)(FILE *, __off_t, int);

        unsigned char *buf;
        size_t         buf_size;

        void *data;
};

#endif /* __RENZAN_OSRT */

typedef unsigned long fpos_t;

enum { __IOFBF = 0, __IOLBF = 1, __IONBF = 2 };

#define _IOFBF __IOFBF
#define _IOLBF __IOLBF
#define _IONBF __IONBF

extern FILE __stderr;
// extern FILE *__stdin;
extern FILE __stdout;

#define stderr (&__stderr)
// #define stdin  (&__stdin)
#define stdout (&__stdout)

/* Operations on files */
/* TODO: want libos filesystem support */
// int   remove(const char *filename);
// int   rename(const char *old, const char *new);
// FILE *tmpfile(void);
// char *tmpnam(char *s);

/* File access functions */
/* TODO: want libos filesystem support */
// int   fclose(FILE *stream);
// int   fflush(FILE *stream);
// FILE *fopen(const char *restrict filename, const char *restrict mode);
// FILE *freopen(
//     const char *restrict filename, const char *restrict mode,
//     FILE *restrict stream);
// void setbuf(FILE *restrict stream, char *restrict buf);
// int  setvbuf(FILE *restrict stream, char *restrict buf, int mode, size_t
// size);

/* Formatted input/output functions */
int fprintf(FILE *restrict stream, const char *restrict format, ...);
int printf(const char *restrict format, ...);
int snprintf(char *restrict s, size_t n, const char *restrict format, ...);
int sprintf(char *restrict s, const char *restrict format, ...);

/* int fscanf(FILE *restrict stream, const char *restrict format, ...); */
/* int scanf(const char *restrict format, ...); */
/* int sscanf(const char *restrict s, const char *restrict format, ...); */

#ifdef _VA_LIST
int vfprintf(FILE *restrict stream, const char *restrict format, va_list arg);
int vprintf(const char *restrict format, va_list arg);
int
vsnprintf(char *restrict s, size_t n, const char *restrict format, va_list arg);
int vsprintf(char *restrict s, const char *restrict format, va_list arg);

/* int vfscanf(FILE *restrict stream, const char *restrict format, va_list arg);
 */
/* int vscanf(const char *restrict format, va_list arg); */
/* int vsscanf(const char *restrict s, const char *restrict format, va_list
 * arg); */
#endif /* _VA_LIST */

/* Character input/output functions */
/* int   fgetc(FILE *stream); */
/* char *fgets(char *restrict s, int n, FILE *restrict stream); */
/* int fputc(int c, FILE *stream); */
/* int fputs(const char *restrict s, FILE *restrict stream); */
/* int   getc(FILE *stream); */
/* int   getchar(void); */
/* int putc(int c, FILE *stream); */
/* int putchar(int c); */
/* int puts(const char *s); */
/* int ungetc(int c, FILE *stream); */

/* Error-handling functions */
void perror(const char *s);

#endif /* __RENZAN_CSTD_STDIO_H__ */
