/* XXX to be removed */
#include <k_cdefs.h>
#include <k_kobj.h>
#include <k_mutex.h>
#include <k_port.h>
#include <k_port_ifce.h>
#include <k_thread.h>
#include <k_string.h>

#include <osrt/error.h>

#define KOBJ_INIT_SIZE 64

boolean
kobject_init(thread_t *proc, int *error)
{
        proc->kobjects = kmem_alloc_zeroed(KOBJ_INIT_SIZE * sizeof(kobject_t));
        if (!proc->kobjects) { *error = ERROR(NOMEM); }
        proc->kobject_size = KOBJ_INIT_SIZE;
        for (size_t i = 0; i < proc->kobject_size; ++i) {
                mutex_init(&proc->kobjects[i].lock);
        }
        return B_TRUE;
}

static boolean
expand_kobjects(thread_t *proc)
{
        size_t new_kobject_size = proc->kobject_size * 3 / 2;
        kobject_t *new =
            kmem_alloc_zeroed(new_kobject_size * sizeof(kobject_t));
        if (!new) { return B_FALSE; }

        for (size_t i = 0; i < proc->kobject_size; ++i) {
                mutex_acquire(&proc->kobjects[i].lock);
                kmemcpy(new + i, proc->kobjects + i, sizeof(kobject_t));
                mutex_release(&proc->kobjects[i].lock);
        }

        for (size_t i = 0; i < new_kobject_size; ++i) {
                mutex_init(&new[i].lock);
        }

        kmem_free(proc->kobjects);
        proc->kobjects     = new;
        proc->kobject_size = new_kobject_size;

        return B_TRUE;
}

/* Assume holding process lock */
kobject_t *
kobject_alloc_lock(thread_t *proc, kobject_handler_t *handler)
{
        kobject_handler_t i;
        for (i = 0; i < proc->kobject_size; ++i) {
                if (proc->kobjects[i].type != KOBJ_UNALLOCED) { continue; }

                kobject_t *ret = &proc->kobjects[i];
                ret->type      = KOBJ_PREALLOCED;
                *handler       = i + 1;
                mutex_acquire(&proc->kobjects[i].lock);

                return ret;
        }

        if (!expand_kobjects(proc)) { return NULL; }

        ASSERT(proc->kobjects[i].type == KOBJ_UNALLOCED);

        proc->kobjects[i].type = KOBJ_PREALLOCED;
        *handler               = i + 1;

        return &proc->kobjects[i];
}

/* Assume holding process lock */
kobject_t *
kobject_lock_fetch(thread_t *proc, kobject_handler_t handler)
{
        if (!handler) { return NULL; }

        if (proc->kobject_size < handler) { return NULL; }

        kobject_t *ret = &proc->kobjects[handler - 1];

        if (ret->type == KOBJ_UNALLOCED) { return NULL; }

        mutex_acquire(&ret->lock);

        return ret;
}

/* Assume holding object lock */
void
kobject_free_release(kobject_t *obj)
{
        obj->type = KOBJ_UNALLOCED;
        obj->ptr  = NULL;

        mutex_release(&obj->lock);
}

void
kobject_cleanup(thread_t *proc)
{
        for (size_t i = 0; i < proc->kobject_size; ++i) {
                kobject_t *obj = &proc->kobjects[i];
                switch (obj->type) {
                case KOBJ_UNALLOCED:
                case KOBJ_PREALLOCED:
                        continue;
                case KOBJ_TYPE_PORT_SERVER_REF:
                        port_server_close(obj->ptr);
                        continue;
                case KOBJ_TYPE_PORT_CLIENT_REF:
                        port_client_close(obj->ptr);
                        continue;
                case KOBJ_TYPE_ADDRESS_SPACE:;
                        address_space_t *as = obj->ptr;
                        mutex_acquire(&as->lock);
                        if (!--as->ref_count) {
                                mutex_release(&as->lock);
                                vm_address_space_destroy(as);
                        }
                        mutex_release(&as->lock);
                        continue;
                case KOBJ_TYPE_PORT_IFCE:;
                        port_ifce_t *ifce = obj->ptr;
                        ifce->close_func();
                        continue;
                }
        }
}
