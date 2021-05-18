/* ctype.h -- Character handling */
#ifndef __RENZAN_CSTD_CTYPE_H__
#define __RENZAN_CSTD_CTYPE_H__

/* Character classification functions */
int isalnum(int c);
int isalpha(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);

/* Character case mapping functions */
int tolower(int c);
int toupper(int c);

#endif /* __RENZAN_CSTD_CTYPE_H__ */
