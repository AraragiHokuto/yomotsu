#include <hal_percpu.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/console.h>
#include <os/kernel/error.h>
#include <os/kernel/futex.h>
#include <os/kernel/list.h>
#include <os/kernel/memory.h>
#include <os/kernel/mutex.h>
#include <os/kernel/port.h>
#include <os/kernel/string.h>
#include <os/kernel/timer.h>
#include <os/kernel/user_memory.h>

struct port_hash_entry_s {
        list_node_t node;

        char *  key;
        port_t *val;
};

typedef struct port_hash_entry_s port_hash_entry_t;

static list_node_t *port_hash;
static size_t       hash_capacity, hash_size;
static mutex_t      hash_lock;

#define PORT_HASH_INIT_CAP    32
#define PORT_HASH_LOAD_FACTOR 75

static void
port_hash_init(void)
{
        port_hash = kmem_alloc(PORT_HASH_INIT_CAP * sizeof(list_node_t));
        VERIFY(port_hash, "failed to alloc port hashmap");

        for (size_t i = 0; i < PORT_HASH_INIT_CAP; ++i) {
                list_head_init(&port_hash[i]);
        }

        hash_capacity = PORT_HASH_INIT_CAP;
        hash_size     = 0;

        mutex_init(&hash_lock);
}

void
_port_hash_do_insert(port_hash_entry_t *entry)
{
        u32 idx = kstr_hash(entry->key, kstrlen(PORT_NAME_MAX_LEN, entry->key))
                  % hash_capacity;
        list_insert(&entry->node, &port_hash[idx]);
}

static port_hash_entry_t *
_port_hash_do_locate(const char *key, size_t keylen)
{
        if (keylen >= PORT_NAME_MAX_LEN) return NULL;

        u32 idx =
            kstr_hash(key, kstrlen(PORT_NAME_MAX_LEN, key)) % hash_capacity;
        for (list_node_t *p = port_hash[idx].next; p != &port_hash[idx];
             p              = p->next) {
                port_hash_entry_t *e = CONTAINER_OF(p, port_hash_entry_t, node);
                if (kstrequ(PORT_NAME_MAX_LEN, key, e->key)) { return e; }
        }

        return NULL;
}

static void
_port_hash_rehash_slot(list_node_t *slot)
{
        LIST_FOREACH_MUT(*slot, p, n)
        {
                port_hash_entry_t *e = CONTAINER_OF(p, port_hash_entry_t, node);
                list_remove(&e->node);
                _port_hash_do_insert(e);
        }
}

static void
_port_hash_rehash(int *error)
{
        list_node_t *old          = port_hash;
        size_t       new_hash_cap = hash_capacity * 3 / 2;

        list_node_t *new = kmem_alloc(new_hash_cap * sizeof(list_node_t));
        if (!new) {
                *error = KERN_ERROR_NOMEM;
                return;
        }

        for (size_t i = 0; i < new_hash_cap; ++i) { list_head_init(&new[i]); }

        size_t old_hash_cap = hash_capacity;
        hash_capacity       = new_hash_cap;

        for (size_t i = 0; i < old_hash_cap; ++i) {
                _port_hash_rehash_slot(&old[i]);
        }
}

static void
port_hash_insert(const char *name, size_t namelen, port_t *val, int *error)
{
        port_hash_entry_t *entry = kmem_alloc(sizeof(port_hash_entry_t));
        if (!entry) {
                *error = KERN_ERROR_NOMEM;
                return;
        }

        entry->key = kmem_alloc(namelen + 1);
        if (!entry->key) {
                *error = KERN_ERROR_NOMEM;
                kmem_free(entry);
                return;
        }
        kmemcpy(entry->key, name, namelen);
        entry->key[namelen] = '\0';

        entry->val = val;

        mutex_acquire(&hash_lock);

        if (_port_hash_do_locate(name, namelen)) {
                mutex_release(&hash_lock);
                kmem_free(entry->key);
                kmem_free(entry);
                *error = KERN_ERROR_PORT_NAME_OCCUPIED;
                return;
        }

        if (hash_size * 100 / hash_capacity >= PORT_HASH_LOAD_FACTOR) {
                int rehash_err = 0;
                _port_hash_rehash(&rehash_err);
                if (rehash_err) {
                        mutex_release(&hash_lock);
                        kmem_free(entry->key);
                        kmem_free(entry);
                        *error = rehash_err;
                        return;
                }
        }

        ++hash_size;
        _port_hash_do_insert(entry);

        mutex_release(&hash_lock);
}

static port_t *
port_hash_remove(const char *name, size_t namelen)
{
        mutex_acquire(&hash_lock);

        port_hash_entry_t *entry = _port_hash_do_locate(name, namelen);
        if (!entry) {
                mutex_release(&hash_lock);
                return NULL;
        }

        list_remove(&entry->node);
        mutex_release(&hash_lock);

        port_t *ret = entry->val;
        kmem_free(entry->key);
        kmem_free(entry);

        return ret;
}

static port_t *
port_hash_fetch(const char *name, size_t namelen)
{
        mutex_acquire(&hash_lock);
        port_hash_entry_t *entry = _port_hash_do_locate(name, namelen);
        mutex_release(&hash_lock);

        return entry ? entry->val : NULL;
}

static port_t *
port_hash_fetch_inc_rc(const char *name, size_t namelen)
{
        mutex_acquire(&hash_lock);
        port_hash_entry_t *entry = _port_hash_do_locate(name, namelen);
        if (entry) {
                atomic_inc_fetch_uint(entry->val->ref_count, __ATOMIC_RELAXED);
        }
        mutex_release(&hash_lock);

        return entry ? entry->val : NULL;
}

void
port_init(void)
{
        port_hash_init();
}

port_server_ref_t *
port_create(const char *name, size_t namelen, int *error)
{
        if (port_hash_fetch(name, namelen) != NULL) {
                *error = KERN_ERROR_PORT_NAME_OCCUPIED;
                return NULL;
        }

        port_server_ref_t *ref = kmem_alloc_zeroed(sizeof(port_server_ref_t));
        if (!ref) {
                *error = KERN_ERROR_NOMEM;
                return NULL;
        }

        port_t *port = kmem_alloc_zeroed(sizeof(port_t));
        if (!port) {
                *error = KERN_ERROR_NOMEM;
                kmem_free(ref);
                return NULL;
        }

        port->name = kmem_alloc(namelen + 1);
        if (!port->name) {
                *error = KERN_ERROR_NOMEM;
                kmem_free(port);
                kmem_free(ref);
                return NULL;
        }

        ref->port   = port;
        ref->holder = CURRENT_PROCESS;

        kmemcpy(port->name, name, namelen);
        port->name[namelen]    = '\0';
        port->ref_count        = 1;
        port->server_ref_count = 1;
        mutex_init(&port->lock);
        list_head_init(&port->server_queue);

        int insert_error = KERN_OK;
        port_hash_insert(name, namelen, port, &insert_error);

        if (insert_error != KERN_OK) {
                kmem_free(port->name);
                kmem_free(port);
                kmem_free(ref);

                *error = insert_error;
                return NULL;
        }

        return ref;
}

static void
_port_free(port_t *port)
{
        ASSERT(port->ref_count == 0);

        kmem_free(port->name);
        kmem_free(port);
}

/* Assume holding port lock */
static port_server_ref_t *
_port_dequeue_server(port_t *port)
{
        if (list_is_empty(&port->server_queue)) { return NULL; }

        port_server_ref_t *server = CONTAINER_OF(
            port->server_queue.next, port_server_ref_t, server_queue_node);
        list_remove(&server->server_queue_node);
        return server;
}

/*
 * Assume server dequeued
 * Assume holding port lock
 */
static void
_port_send_request(port_server_ref_t *server, port_request_t *req)
{
#ifdef _KDEBUG
        void *p = atomic_xchg_ptr(server->req, req, __ATOMIC_ACQ_REL);
        ASSERT(!p);
#else
        atomic_store_ptr(server->req, req, __ATOMIC_RELEASE);
#endif
        futex_kwake((void *)&server->req, 1);
}

port_client_ref_t *
port_open(const char *name, size_t namelen, int *error)
{
        port_client_ref_t *ref = kmem_alloc_zeroed(sizeof(port_client_ref_t));
        if (!ref) {
                *error = KERN_ERROR_NOMEM;
                return NULL;
        }
        ref->holder = CURRENT_PROCESS;

        port_t *port = port_hash_fetch_inc_rc(name, namelen);
        if (!port) {
                *error = KERN_ERROR_NOENT;
                kmem_free(ref);
                return NULL;
        }
        ref->port = port;

        kmemset(&ref->req, 0, sizeof(port_request_t));
        return ref;
}

void
port_server_close(port_server_ref_t *ref)
{
        if (ref->req) {
                port_request_t *req =
                    atomic_xchg_ptr(ref->req, NULL, __ATOMIC_ACQ_REL);
                req->error = KERN_ERROR_PORT_CANCELED;
                atomic_store_boolean(req->finished, B_TRUE, __ATOMIC_SEQ_CST);
                futex_kwake(&req->finished, 1);
                ref->req = NULL;
        }

        uint recv_rc = atomic_dec_fetch_uint(
            ref->port->server_ref_count, __ATOMIC_ACQ_REL);
        uint rc = atomic_dec_fetch_uint(ref->port->ref_count, __ATOMIC_ACQ_REL);

        if (!recv_rc) {
                port_hash_remove(
                    ref->port->name,
                    kstrlen(PORT_NAME_MAX_LEN, ref->port->name));
                futex_kwake(&ref->port->waiting_server_count, FUTEX_WAKE_ALL);
        }

        if (!rc) {
                _port_free(ref->port);
                return;
        }

        kmem_free(ref);
}

void
port_client_close(port_client_ref_t *ref)
{
        kmem_free(ref);
}

/* Assume request verified */
void
port_request(port_client_ref_t *ref, int *error)
{
        port_server_ref_t *serv = NULL;
        while (!serv) {
                if (!atomic_load_uint(
                        ref->port->server_ref_count, __ATOMIC_ACQUIRE)) {
                        /* all server have closed port */
                        *error = KERN_ERROR_PORT_CLOSED;
                        return;
                }

                if (!atomic_load_uint(
                        ref->port->waiting_server_count, __ATOMIC_ACQUIRE)) {
                        /* block until there's server waiting */
                        futex_kwait(&ref->port->waiting_server_count, 0);
                }

                mutex_acquire(&ref->port->lock);
                serv = _port_dequeue_server(ref->port);

                if (!serv) mutex_release(&ref->port->lock);
        }

        _port_send_request(serv, &ref->req);
        mutex_release(&ref->port->lock);

        if (LIKELY(!atomic_load_boolean(ref->req.finished, __ATOMIC_SEQ_CST))) {
                futex_kwait(&ref->req.finished, B_FALSE);
        }

        if (atomic_xchg_ptr(serv->req, NULL, __ATOMIC_SEQ_CST)) {
                /* serv->req was not NULL, indicate a timeout */
                *error = -KERN_ERROR_TIMEOUT;
                return;
        } else {
                /* serv->req was already NULL, response en route */
                futex_kwait(&ref->req.finished, B_FALSE);
        }

        if (ref->req.error != KERN_OK) { *error = ref->req.error; }

        return;
}

port_request_t *
port_receive(port_server_ref_t *ref, void *buf, size_t buflen)
{
        ASSERT(!atomic_load_ptr(ref->req, __ATOMIC_ACQUIRE));

        while (B_TRUE) {
                /* enqueue */
                mutex_acquire(&ref->port->lock);
                ref->port->waiting_server_count++;
                list_insert(
                    &ref->server_queue_node, ref->port->server_queue.prev);
                mutex_release(&ref->port->lock);

                futex_kwake(&ref->port->waiting_server_count, 1);

                /* TODO: timeout */
                if (LIKELY(!ref->req)) { futex_kwait((void *)&ref->req, 0); }

                port_request_t *ret = ref->req;

                if (ret->data_size > buflen) {
                        /* Data too large; refuse this one */
                        ret->error = KERN_ERROR_PORT_DATA_TOO_LONG;
                        atomic_xchg_ptr(ref->req, NULL, __ATOMIC_SEQ_CST);
                        atomic_store_boolean(
                            ret->finished, B_TRUE, __ATOMIC_SEQ_CST);
                        futex_kwake(&ret->finished, 1);
                        continue;
                }

                if (ret->data_sender_vaddr) {
                        int copy_ret = user_memory_copy(
                            CURRENT_ADDRESS_SPACE, buf,
                            ret->sender->holder->address_space,
                            ret->data_sender_vaddr, ret->data_size);
                        if (UNLIKELY(copy_ret != 0)) {
                                atomic_xchg_ptr(
                                    ref->req, NULL, __ATOMIC_SEQ_CST);
                                if (copy_ret == 1) {
                                        mutex_acquire(
                                            &ret->sender->holder->lock);
                                        process_raise_exception(
                                            ret->sender->holder,
                                            PROC_EXCEPTION_ACCESS_VIOLATION);
                                        mutex_release(
                                            &ret->sender->holder->lock);
                                } else {
                                        mutex_acquire(&CURRENT_PROCESS->lock);
                                        process_raise_exception(
                                            ret->sender->holder,
                                            PROC_EXCEPTION_ACCESS_VIOLATION);
                                        mutex_release(&CURRENT_PROCESS->lock);
                                }
                        }
                }

                return ret;
        }
}

void
port_response(
    port_server_ref_t *ref, u64 retval_small, void *retval, size_t retval_size,
    int *error)
{
        port_request_t *req = atomic_xchg_ptr(ref->req, NULL, __ATOMIC_SEQ_CST);
        if (!req) {
                *error = KERN_ERROR_PORT_CANCELED;
                return;
        }
        ASSERT(!atomic_load_boolean(req->finished, __ATOMIC_ACQUIRE));

        req->val_small = retval_small;

        if (retval && retval_size > req->retval_size) {
                *error = KERN_ERROR_INVAL;
                return;
        }

        if (retval) {
                int copy_ret = user_memory_copy(
                    req->sender->holder->address_space,
                    req->retval_sender_vaddr, CURRENT_ADDRESS_SPACE, retval,
                    retval_size);
                if (UNLIKELY(copy_ret != 0)) {
                        if (copy_ret == 1) {
                                mutex_acquire(&req->sender->holder->lock);
                                process_raise_exception(
                                    req->sender->holder,
                                    PROC_EXCEPTION_ACCESS_VIOLATION);
                                mutex_release(&req->sender->holder->lock);
                                *error = KERN_ERROR_PORT_CANCELED;
                                return;
                        } else {
                                mutex_acquire(&CURRENT_PROCESS->lock);
                                process_raise_exception(
                                    CURRENT_PROCESS,
                                    PROC_EXCEPTION_ACCESS_VIOLATION);
                                mutex_release(&CURRENT_PROCESS->lock);
                                return;
                        }
                }
        }

        atomic_store_boolean(req->finished, B_TRUE, __ATOMIC_SEQ_CST);
        futex_kwake(&req->finished, 1);
}
