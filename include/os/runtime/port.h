/* port.h -- port system wrapping */
#ifndef __ORIHIME_LIBOS_PORT_H__
#define __ORIHIME_LIBOS_PORT_H__

#include <os/kernel/kobject.h>
#include <os/kernel/port.h>

typedef kobject_t port_t;

port_t port_create(const char *port_name);
port_t port_open(const char *port_name);
int    port_close(port_t port);
int    port_request(port_t port, port_request_t *request);
int    port_receive(
       port_t port, port_request_t *buffer, void *data_buffer, size_t buffer_size);
int port_response(
    kobject_t request, uint64_t retval, void *ret_data, size_t ret_data_size);

#endif /* __ORIHIME_LIBOS_PORT_H__ */
