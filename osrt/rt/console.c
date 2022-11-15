/* XXX to be reimplemented */

#include <errno.h>
#include <string.h>

#include <osrt/console.h>
#include <osrt/syscall.h>

#define DEBUG_PRINT_PORT_NAME "kern.debug_print"

void
console_write(const char *str, size_t str_size)
{
#ifdef __RZ_DEBUG
	syscall_debug_print(str, str_size);
#endif	/* __RZ_DEBUG */
}
