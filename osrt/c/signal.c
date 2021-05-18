/* signal.c -- Signal handling */
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

typedef void (*signal_handler_t)(int);

static signal_handler_t __signal_handlers[__SIGNAL_MAX];

signal_handler_t
signal(int sig, signal_handler_t handler) {
	if (sig >= __SIGNAL_MAX) {
		errno = EINVAL;
		return SIG_ERR;
	}

	signal_handler_t ret = __signal_handlers[sig];
	__signal_handlers[sig] = handler;
	return ret;
}

int
raise(int sig)
{
	signal_handler_t ret = __signal_handlers[sig];
	if (ret == SIG_DFL) {
		exit(EXIT_FAILURE);
	} else if (ret == SIG_IGN) {
		return 0;
	} else {
		ret(sig);
		return 0;
	}
}
