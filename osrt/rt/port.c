#include <errno.h>
#include <string.h>

#include <osrt/port.h>
#include <osrt/syscall.h>

port_t
port_create(const char *port_name)
{
        kobject_t ret =
            syscall_port_create((char *)port_name, strlen(port_name));
        if (ret < 0) {
                errno = -ret;
                return -1;
        }
        return ret;
}

port_t
port_open(const char *port_name)
{
        kobject_t ret = syscall_port_open((char *)port_name, strlen(port_name));

        if (ret < 0) {
                errno = -ret;
                return -1;
        }
        return ret;
}

int
port_close(port_t port)
{
        kobject_t ret = syscall_port_close(port);
        if (ret < 0) {
                errno = -ret;
                return -1;
        }

        return ret;
}

int
port_request(port_t port, port_request_t *request)
{
        int64_t ret = syscall_port_request(port, request);

        if (ret < 0) {
                errno = -ret;
                return -1;
        }

        return 0;
}

int
port_receive(
    port_t port, port_request_t *buffer, void *data_buffer, size_t buffer_size)
{
        int64_t ret =
            syscall_port_receive(port, buffer, data_buffer, buffer_size);
        if (ret < 0) {
                errno = -ret;
                return -1;
        }

        return 0;
}

int
port_response(
    kobject_t request, uint64_t retval, void *ret_data, size_t ret_data_size)
{
        int64_t ret =
            syscall_port_response(request, retval, ret_data, ret_data_size);
        if (ret < 0) {
                errno = -ret;
                return -1;
        }

        return 0;
}
