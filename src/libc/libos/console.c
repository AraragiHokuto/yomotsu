#include <errno.h>
#include <libos/console.h>
#include <libos/syscall.h>
#include <string.h>

#define DEBUG_PRINT_PORT_NAME "kern.debug_print"

void
console_write(const char *str, size_t str_size)
{
        /* stub */
        kobject_t port = syscall_port_open(
            DEBUG_PRINT_PORT_NAME, strlen(DEBUG_PRINT_PORT_NAME));
        if (port < 0) {
                errno = -port;
                return;
        }

        port_request_t request = {
            .data_addr   = (void *)str,
            .data_size   = str_size,
            .retval_addr = NULL,
            .retval_size = 0};

        int ret = syscall_port_request(port, &request);
        if (ret < 0) {
                errno = -ret;
                return;
        }

        syscall_port_close(port);
}
