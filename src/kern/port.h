#ifndef IZANAMI_PORT_H__
#define IZANAMI_PORT_H__

#include <kern/boolean.h>
#include <kern/process.h>

#define PORT_NAME_MAX_LEN 128

#define PORT_REQ_OPEN_ACCEPT _B_TRUE
#define PORT_REQ_OPEN_REJECT _B_FALSE

enum PORT_MSG_TYPES {
        PORT_REQ_TYPE_OPEN         = 1,
        PORT_REQ_TYPE_CLOSE        = 2,
        PORT_REQ_TYPE_CUSTOM_START = 0xff,
};

#ifdef _KERNEL

typedef u64 __u64;

#include <kern/atomic.h>
#include <kern/kobject.h>
#include <kern/list.h>
#include <kern/mutex.h>

typedef struct __port_request_s port_request_user_t;

typedef struct port_server_ref_s port_server_ref_t;
typedef struct port_client_ref_s port_client_ref_t;

struct port_request_s {
        port_client_ref_t *sender;

        u64 val_small;

        void * data_sender_vaddr;
        size_t data_size;

        void * retval_sender_vaddr;
        size_t retval_size;

        atomic_boolean finished;
        int            error;
};

typedef struct port_request_s port_request_t;

struct port_s {
        char *      name;
        atomic_uint ref_count;
        atomic_uint server_ref_count;
        atomic_uint waiting_server_count;

        mutex_t     lock;
        list_node_t server_queue;
};

typedef struct port_s port_t;

struct port_server_ref_s {
        port_t *   port;
        process_t *holder;

        list_node_t server_queue_node;
        atomic_ptr  req;
};

struct port_client_ref_s {
        port_t *   port;
        process_t *holder;

        port_request_t req;
};

void               port_init(void);
port_server_ref_t *port_create(const char *name, size_t namelen, int *error);
port_client_ref_t *port_open(const char *name, size_t namelen, int *error);
void               port_server_close(port_server_ref_t *ref);
void               port_client_close(port_client_ref_t *ref);
void               port_request(port_client_ref_t *ref, int *error);
port_request_t *port_receive(port_server_ref_t *ref, void *buf, size_t buflen);
void            port_response(
               port_server_ref_t *ref, u64 retval_small, void *retval, size_t retval_size,
               int *error);

#else /* _KERNEL */

#include <stddef.h>
#include <stdint.h>

typedef struct __port_request_s port_request_t;

#endif /* _KERNEL */

struct __port_request_s {
        pid_t sender_pid;

        __u64 val_small;

        void * data_addr;
        size_t data_size;

        void * retval_addr;
        size_t retval_size;
};

#endif /* IZANAMI_PORT_H__ */
