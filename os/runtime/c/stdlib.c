#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

/** Numeric conversion functions **/

/* int */
/* atoi(const char *nptr) */
/* { */
/* 	return strtol(nptr, NULL, 10); */
/* } */

/* long int */
/* atol(const char *nptr) */
/* { */
/* 	return strtol(nptr, NULL, 10); */
/* } */

/* long long int */
/* atoll(const char *nptr) */
/* { */
/* 	return strtoll(nptr, NULL, 10); */
/* } */

/** Pseudo-random sequence generator **/

/* xorshift32 random generator */
/* TODO: thread local */
uint32_t __rand_state = 1;

int
rand(void)
{
        uint32_t x = __rand_state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return __rand_state = x;
}

void
srand(uint32_t seed)
{
        __rand_state = seed;
}

/** Communication with the environment **/
__attribute__((noreturn)) void
abort(void)
{
        raise(SIGABRT);
	_Exit(EXIT_FAILURE);
}

/* TODO: thread safety? */
typedef void (*atexit_func_t)(void);
#define INIT_ATEXIT_SIZE 32
static atexit_func_t  __init_atexit_func[INIT_ATEXIT_SIZE];
static atexit_func_t *__atexit_func       = __init_atexit_func;
static size_t         __atexit_func_count = 0;
static size_t         __atexit_func_size  = INIT_ATEXIT_SIZE;

int
atexit(atexit_func_t func)
{
        if (__atexit_func_count == __atexit_func_size) {
                /* realloc atexit table */
                size_t new_size = __atexit_func_size * 3 / 2;
                if (__atexit_func == __init_atexit_func) {
                        __atexit_func =
                            malloc(sizeof(atexit_func_t) * new_size);
                        if (!__atexit_func) {
                                __atexit_func = __init_atexit_func;
                                errno         = ENOMEM;
                                return -1;
                        }
                } else {
                        atexit_func_t *new_table = realloc(
                            __atexit_func, sizeof(atexit_func_t) * new_size);
                        if (!new_table) {
                                errno = ENOMEM;
                                return -1;
                        }
                        __atexit_func = new_table;
                }
                __atexit_func_size = new_size;
        }

        __atexit_func[__atexit_func_count++] = func;
        return 0;
}

static atexit_func_t  __init_at_quick_exit_func[INIT_ATEXIT_SIZE];
static atexit_func_t *__at_quick_exit_func       = __init_at_quick_exit_func;
static size_t         __at_quick_exit_func_count = 0;
static size_t         __at_quick_exit_func_size  = INIT_ATEXIT_SIZE;

int
at_quick_exit(void (*func)(void))
{
        if (__at_quick_exit_func_count == __at_quick_exit_func_size) {
                /* realloc at_quick_exit func table */
                size_t new_size = __atexit_func_size * 3 / 2;
                if (__at_quick_exit_func == __init_at_quick_exit_func) {
                        __at_quick_exit_func =
                            malloc(sizeof(atexit_func_t) * new_size);
                        if (!__at_quick_exit_func) {
                                __at_quick_exit_func =
                                    __init_at_quick_exit_func;
                                errno = ENOMEM;
                                return -1;
                        }
                } else {
                        atexit_func_t *new_table = realloc(
                            __at_quick_exit_func,
                            sizeof(atexit_func_t) * new_size);
                        if (!new_table) {
                                errno = ENOMEM;
                                return -1;
                        }
                        __at_quick_exit_func = new_table;
                }
                __at_quick_exit_func_size = new_size;
        }

        __at_quick_exit_func[__at_quick_exit_func_count++] = func;
        return 0;
}

__attribute__((noreturn)) void
exit(int status)
{
	/* invoke atexit functions */
	for (size_t i = __atexit_func_count; i > 0; --i) {
		__atexit_func[i - 1]();
	}

	/* TODO: flush streams */

	_Exit(status);
}

__attribute__((noreturn)) void __exit(int64_t retval);

__attribute__((noreturn)) void
_Exit(int status)
{
	__exit(status);
}

char *
getenv(const char *name)
{
	/* TODO: want libos env support */
	(void)name;
	return NULL;
}

__attribute__((noreturn)) void
quick_exit(int status)
{
	for (size_t i = __at_quick_exit_func_count; i > 0; --i) {
		__at_quick_exit_func[i - 1]();
	}

	_Exit(status);
}

int
system(const char *string)
{
	/* TODO: want libos process spawn support */
	/* TODO: want libos fs support */

	return string ? -1 : 0;
}

int
abs(int j)
{
	return j < 0 ? -j : j;
}

long int
labs(long int j)
{
	return j < 0 ? -j : j;
}

long long int
llabs(long long int j)
{
	return j < 0 ? -j : j;
}

div_t
div(int numer, int denom)
{
	div_t ret;
	ret.quot = numer / denom;
	ret.rem = numer % denom;
	return ret;
}

ldiv_t
ldiv(long int numer, long int denom)
{
	ldiv_t ret;
	ret.quot = numer / denom;
	ret.rem = numer % denom;
	return ret;
}

lldiv_t
lldiv(long long int numer, long long int denom)
{
	lldiv_t ret;
	ret.quot = numer / denom;
	ret.rem = numer % denom;
	return ret;
}
