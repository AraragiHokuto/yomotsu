/* port.h -- port system wrapping */
/* XXX to be reimplemented */

#ifndef __RENZAN_OSRT_PORT_H__
#define __RENZAN_OSRT_PORT_H__

#include <osrt/kobject.h>
#include <osrt/process.h>
#include <osrt/types.h>

struct __osrt_port_request_s {
        __osrt_pid_t sender_pid;

        __osrt_u64 val_small;

        void * data_addr;
        size_t data_size;

        void * retval_addr;
        size_t retval_size;
};

#define PORT_NAME_MAX_LEN 128

#define PORT_REQ_OPEN_ACCEPT _B_TRUE
#define PORT_REQ_OPEN_REJECT _B_FALSE

enum PORT_MSG_TYPES {
        PORT_REQ_TYPE_OPEN         = 1,
        PORT_REQ_TYPE_CLOSE        = 2,
        PORT_REQ_TYPE_CUSTOM_START = 0xff,
};

#ifndef __RZ_KERNEL

typedef kobject_t port_t;

typedef struct __osrt_port_request_s port_request_t;

port_t port_create(const char *port_name);
port_t port_open(const char *port_name);
int    port_close(port_t port);
int    port_request(port_t port, port_request_t *request);
int    port_receive(
       port_t port, port_request_t *buffer, void *data_buffer, size_t buffer_size);
int port_response(
    kobject_t request, uint64_t retval, void *ret_data, size_t ret_data_size);

#endif /* __RZ_KERNEL */

#endif /* __RENZAN_OSRT_PORT_H__ */
