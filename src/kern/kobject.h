#ifndef IZANAMI_KOBJECT_H__
#define IZANAMI_KOBJECT_H__

#include <kern/atomic.h>
#include <kern/mutex.h>
#include <kern/process.h>
#include <kern/types.h>

#ifdef _KERNEL

typedef size_t kobject_handler_t;

enum KOBJ_TYPE {
        KOBJ_UNALLOCED  = 0,
        KOBJ_PREALLOCED = 1,
        KOBJ_TYPE_PORT_SERVER_REF,
        KOBJ_TYPE_PORT_CLIENT_REF,
        KOBJ_TYPE_PORT_REQUEST,
        KOBJ_TYPE_ADDRESS_SPACE,
        KOBJ_TYPE_PORT_IFCE,
};

struct kobject_s {
        u32     type;
        mutex_t lock;
        void *  ptr;
};

boolean    kobject_init(process_t *proc, int *error);
kobject_t *kobject_alloc_lock(process_t *proc, kobject_handler_t *handler);
kobject_t *kobject_lock_fetch(process_t *proc, kobject_handler_t handler);
void       kobject_free_release(kobject_t *object);
void       kobject_cleanup(process_t *proc);

#else

typedef unsigned long kobject_handler_t;

#endif /* _KERNEL */

#endif /* IZANAMI_KOBJECT_H__ */
