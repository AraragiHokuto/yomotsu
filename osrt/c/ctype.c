/* ctype.c -- Character handling */
/* TODO locale */

#include <ctype.h>

/** Character classification functions **/
int
isalnum(int c)
{
        return isalpha(c) || isdigit(c);
}

int
isalpha(int c)
{
        return islower(c) || isupper(c);
}

int
isblank(int c)
{
        return c == ' ' || c == '\t';
}

int
iscntrl(int c)
{
        return c <= 0x1f || c == 0x7f;
}

int
isdigit(int c)
{
        return c >= '0' && c <= '9';
}

int
isgraph(int c)
{
        return isprint(c) && c != ' ';
}

int
islower(int c)
{
        return c >= 'a' && c <= 'z';
}

int
isprint(int c)
{
        return !iscntrl(c);
}

int
ispunct(int c)
{
        return isprint(c) && !isspace(c) && !isalnum(c);
}

int
isspace(int c)
{
        return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t'
               || c == '\v';
}

int
isupper(int c)
{
        return c >= 'A' && c <= 'Z';
}

int
isxdigit(int c)
{
        return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

/** Character case mapping functions **/
int
tolower(int c)
{
        return isupper(c) ? (c - 'A' + 'a') : c;
}

int
toupper(int c)
{
        return islower(c) ? (c - 'a' + 'A') : c;
}
