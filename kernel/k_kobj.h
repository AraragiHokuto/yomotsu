/* XXX to be removed */
#ifndef RENZAN_KOBJECT_H__
#define RENZAN_KOBJECT_H__

#ifdef _KERNEL

#include <k_atomic.h>
#include <k_mutex.h>
#include <k_proc.h>

#include <osrt/kobject.h>
#include <osrt/types.h>

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

#else /* _KERNEL */

#include <stdint.h>
typedef int64_t kobject_t;

#endif /* _KERNEL */

#endif /* RENZAN_KOBJECT_H__ */
