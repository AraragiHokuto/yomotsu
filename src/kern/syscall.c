#include <kern/asm.h>
#include <kern/console.h>
#include <kern/error.h>
#include <kern/futex.h>
#include <kern/macrodef.h>
#include <kern/memory.h>
#include <kern/percpu.h>
#include <kern/port.h>
#include <kern/port_ifce.h>
#include <kern/string.h>
#include <kern/syscall.h>
#include <kern/user_memory.h>

s64
syscall_as_create(void)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_handler_t h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_PROCESS, &h);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!obj) { return -KERN_ERROR_NOMEM; }

        address_space_t *as = vm_address_space_create();
        if (!as) {
                kobject_free_release(obj);
                return -KERN_ERROR_NOMEM;
        }

        ++as->ref_count;

        obj->type = KOBJ_TYPE_ADDRESS_SPACE;
        obj->ptr  = as;

        mutex_release(&obj->lock);
        return h;
}

static address_space_t *
as_lock_fetch(kobject_handler_t h)
{
        if (h == 0) {
                mutex_acquire(&CURRENT_ADDRESS_SPACE->lock);
                return CURRENT_ADDRESS_SPACE;
        }

        kobject_t *obj = kobject_lock_fetch(CURRENT_PROCESS, h);

        if (!obj) return NULL;

        if (obj->type != KOBJ_TYPE_ADDRESS_SPACE) {
                mutex_release(&obj->lock);
                return NULL;
        }

        address_space_t *ret = obj->ptr;
        mutex_acquire(&ret->lock);
        mutex_release(&obj->lock);

        return ret;
}

s64
syscall_as_clone(kobject_handler_t src_h)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        address_space_t *src = as_lock_fetch(src_h);
        if (!src) {
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_INVAL;
        }

        kobject_handler_t ret_h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_PROCESS, &ret_h);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!obj) {
                mutex_release(&src->lock);
                return -KERN_ERROR_NOMEM;
        }

        address_space_t *cloned = vm_address_space_clone(src);
        if (!cloned) {
                mutex_release(&src->lock);
                kobject_free_release(obj);
                return -KERN_ERROR_NOMEM;
        }
        mutex_release(&src->lock);

        ++cloned->ref_count;

        obj->type = KOBJ_TYPE_ADDRESS_SPACE;
        obj->ptr  = cloned;

        mutex_release(&obj->lock);
        return ret_h;
}

s64
syscall_as_destroy(kobject_handler_t h)
{
        if (h == 0) {
                /* can not destroy current address space */
                return -KERN_ERROR_INVAL;
        }

        mutex_acquire(&CURRENT_PROCESS->lock);
        address_space_t *as = as_lock_fetch(h);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!as) { return -KERN_ERROR_INVAL; }

        if (!(--as->ref_count)) {
                mutex_acquire(&CURRENT_PROCESS->lock);

                kobject_t *obj = kobject_lock_fetch(CURRENT_PROCESS, h);

                mutex_release(&as->lock);
                vm_address_space_destroy(as);

                kobject_free_release(obj);

                mutex_release(&CURRENT_PROCESS->lock);
        } else {
                mutex_release(&as->lock);
        }

        return KERN_OK;
}

s64
syscall_mmap(
    kobject_handler_t as_handler, uintptr vaddr, size_t size, uint flag)
{
        if ((vaddr & (1ull << 47)) != 0) {
                /* can not map kernel space */
                return -KERN_ERROR_UNAUTH;
        }

        if (((vaddr + size) & (1ull << 47)) != 0) {
                /* can not map kernel space */
                return -KERN_ERROR_UNAUTH;
        }

        if (flag & ~USER_ALLOWED_FLAGS) { return -KERN_ERROR_UNAUTH; }

        mutex_acquire(&CURRENT_PROCESS->lock);
        address_space_t *as = as_lock_fetch(as_handler);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!vaddr)
                vaddr = (uintptr)vm_alloc_vaddr(
                    as, USERLAND_VADDR_BEGIN, USERLAND_VADDR_END, size);
        if (!vaddr) {
                mutex_release(&as->lock);
                return -KERN_ERROR_NOMEM;
        }

        if (VM_OFFSET_PAGE_2M(vaddr) != 0) {
                mutex_release(&as->lock);
                return -KERN_ERROR_INVAL;
        }

        uintptr vi = vaddr;
        for (size_t mapped = 0; mapped < size; mapped += KMEM_PAGE_SIZE) {
                uintptr paddr = pma_alloc(PMA_ZONE_ANY);
                if (!paddr) {
                        vm_unmap_n(as, (void *)vaddr, mapped);
                        mutex_release(&as->lock);
                        return -KERN_ERROR_NOMEM;
                }

                if (vm_is_mapped(as, (void *)vi)) {
                        vm_unmap_n(as, (void *)vaddr, mapped);
                        mutex_release(&as->lock);
                        return -KERN_ERROR_INVAL;
                }

                if (!vm_map_page(as, (void *)vi, paddr, flag)) {
                        vm_unmap_n(as, (void *)vi, mapped);
                        mutex_release(&as->lock);
                        return -KERN_ERROR_NOMEM;
                }

                kmemset(vm_get_vma_for_pma(paddr), 0, KMEM_PAGE_SIZE);

                vi += KMEM_PAGE_SIZE;
        }

        mutex_release(&as->lock);
        return vaddr;
}

s64
syscall_mtransfer(
    kobject_handler_t dh, kobject_handler_t sh, uintptr vaddr_d,
    uintptr vaddr_s, size_t size, uint flag)
{
        if (!vaddr_s && !vaddr_d) { return -KERN_ERROR_INVAL; }

        if (vaddr_s >= (uintptr)USERLAND_VADDR_END) {
                return -KERN_ERROR_UNAUTH;
        }

        if (vaddr_d >= (uintptr)USERLAND_VADDR_END) {
                kprintf("vaddr_d = %x\n", vaddr_d);
                return -KERN_ERROR_UNAUTH;
        }

        if ((vaddr_s + size) > (uintptr)USERLAND_VADDR_END) {
                return -KERN_ERROR_UNAUTH;
        }

        if ((vaddr_d + size) > (uintptr)USERLAND_VADDR_END) {
                kprintf(
                    "vaddr_d + size = %x, USERLAND_VADDR_END = %x\n",
                    vaddr_d + size, USERLAND_VADDR_END);
                return -KERN_ERROR_UNAUTH;
        }

        if (vaddr_s < (uintptr)USERLAND_VADDR_BEGIN) {
                kprintf("mtransfer: src invalid\n");
                return -KERN_ERROR_INVAL;
        }

        if (vaddr_d < (uintptr)USERLAND_VADDR_BEGIN) {
                kprintf("mtransfer: dst invalid: %d\n", vaddr_d);
                return -KERN_ERROR_INVAL;
        }

        if (VM_OFFSET_PAGE_2M(vaddr_s) != 0) {
                kprintf("mtransfer: src not aligned\n");
                return -KERN_ERROR_INVAL;
        }

        if (VM_OFFSET_PAGE_2M(vaddr_d) != 0) {
                kprintf("mtransfer: dst not aligned: %x\n", vaddr_d);
                return -KERN_ERROR_INVAL;
        }

        if (flag & ~USER_ALLOWED_FLAGS) {
                kprintf(
                    "flag = %x, flag & ~USER_ALLOWED_FLAGS: %x\n", flag,
                    flag & ~USER_ALLOWED_FLAGS);
                return -KERN_ERROR_UNAUTH;
        }

        mutex_acquire(&CURRENT_PROCESS->lock);
        address_space_t *src = as_lock_fetch(sh);
        if (!src) {
                kprintf("mtransfer: failed to fetch src\n");
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_INVAL;
        }

        address_space_t *dst = as_lock_fetch(dh);
        if (!dst) {
                kprintf("mtransfer: failed to fetch dst\n");
                mutex_release(&src->lock);
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_INVAL;
        }

        if (!vaddr_d) {
                /* allocate virtual address for dst */
                vaddr_d = (uintptr)vm_alloc_vaddr(
                    dst, USERLAND_VADDR_BEGIN, USERLAND_VADDR_END, size);
                if (!vaddr_d) {
                        mutex_release(&dst->lock);
                        mutex_release(&src->lock);
                        mutex_release(&CURRENT_PROCESS->lock);
                        return -KERN_ERROR_NOMEM;
                }
        }
        mutex_release(&CURRENT_PROCESS->lock);

        uintptr si = vaddr_s;
        uintptr di = vaddr_d;
        for (size_t mapped = 0; mapped < size; mapped += KMEM_PAGE_SIZE) {
                if (!vm_is_mapped(src, (void *)si)) {
                        kprintf("mtransfer: src not mapped\n");
                        vm_unmap_n(dst, (void *)vaddr_d, mapped);
                        mutex_release(&src->lock);
                        mutex_release(&dst->lock);
                        return -KERN_ERROR_INVAL;
                }

                if (vm_is_mapped(dst, (void *)di)) {
                        kprintf("mtransfer: dst already mapped\n");
                        vm_unmap_n(dst, (void *)vaddr_d, mapped);
                        mutex_release(&src->lock);
                        mutex_release(&dst->lock);
                        return -KERN_ERROR_INVAL;
                }

                uintptr pma = vm_get_pma_by_vma(
                    vm_get_vma_for_pma(src->pml4_paddr), (void *)si);
                pma_inc_ref_count(pma, KMEM_PAGE_SIZE);
                void *map_ret = vm_map_page(dst, (void *)di, pma, flag);
                if (!map_ret) {
                        vm_unmap_n(dst, (void *)vaddr_d, mapped);
                        mutex_release(&src->lock);
                        mutex_release(&dst->lock);
                        return -KERN_ERROR_NOMEM;
                }

                si += KMEM_PAGE_SIZE;
                di += KMEM_PAGE_SIZE;
        }

        vm_unmap_n(src, (void *)vaddr_s, size);

        mutex_release(&src->lock);
        mutex_release(&dst->lock);
        return vaddr_d;
}

s64
syscall_munmap(kobject_handler_t h, uintptr vaddr, size_t size)
{
        /* TODO: privilege check */

        /* TODO: privilege check */
        if ((vaddr & (1ull << 47)) != 0) { return -KERN_ERROR_UNAUTH; }

        if (((vaddr + size) & (1ull << 47)) != 0) {
                /* can not unmap kernel space */
                return -KERN_ERROR_UNAUTH;
        }

        address_space_t *as = as_lock_fetch(h);
        if (!as) { return -KERN_ERROR_INVAL; }

        for (size_t vi = vaddr; vi < (vaddr + size); vi += KMEM_PAGE_SIZE) {
                if (!vm_is_mapped(as, (void *)vi)) {
                        mutex_release(&as->lock);
                        return -KERN_ERROR_UNAUTH;
                }
        }

        vm_unmap_n(as, (void *)vaddr, size);
        mutex_release(&as->lock);
        return 0;
}

s64
syscall_port_create(char *name, size_t namelen)
{
        if (namelen >= PORT_NAME_MAX_LEN) { return -KERN_ERROR_INVAL; }

        char namebuf[PORT_NAME_MAX_LEN];
        if (!user_memory_read(CURRENT_ADDRESS_SPACE, namebuf, name, namelen)) {
                /* TODO: kill process */
                PANIC("unimplemented error handling");
        }
        namebuf[namelen] = '\0';

        mutex_acquire(&CURRENT_PROCESS->lock);
        int               error = KERN_OK;
        kobject_handler_t h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_PROCESS, &h);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!obj) { return -KERN_ERROR_NOMEM; }

        port_server_ref_t *ref = port_create(namebuf, namelen, &error);

        if (!ref) {
                kobject_free_release(obj);
                return -error;
        }

        obj->type = KOBJ_TYPE_PORT_SERVER_REF;
        obj->ptr  = ref;

        mutex_release(&obj->lock);

        return h;
}

s64
syscall_port_open(char *name, size_t namelen)
{
        if (namelen >= PORT_NAME_MAX_LEN) { return -KERN_ERROR_INVAL; }

        char namebuf[PORT_NAME_MAX_LEN];
        if (!user_memory_read(CURRENT_ADDRESS_SPACE, namebuf, name, namelen)) {
                /* TODO: kill process */
                PANIC("unimplemented error handling");
        }
        namebuf[namelen] = '\0';

        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_handler_t h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_PROCESS, &h);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!obj) { return -KERN_ERROR_NOMEM; }

        /* check if we have a port-like interface */
        port_ifce_t *ifce = port_ifce_lookup(namebuf);
        if (ifce) {
                sint error = ifce->open_func();
                if (error != KERN_OK) {
                        kobject_free_release(obj);
                        return -error;
                }

                obj->type = KOBJ_TYPE_PORT_IFCE;
                obj->ptr  = ifce;

                mutex_release(&obj->lock);
                return h;
        }

        /* normal port open */
        int                error = KERN_OK;
        port_client_ref_t *ref   = port_open(namebuf, namelen, &error);

        if (!ref) {
                kobject_free_release(obj);
                return -error;
        }

        obj->type = KOBJ_TYPE_PORT_CLIENT_REF;
        obj->ptr  = ref;

        mutex_release(&obj->lock);

        return h;
}

s64
syscall_port_close(kobject_handler_t handler)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_t *obj = kobject_lock_fetch(CURRENT_PROCESS, handler);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!obj) { return -KERN_ERROR_INVAL; }

        if (obj->type == KOBJ_TYPE_PORT_SERVER_REF) {
                port_server_close(obj->ptr);
                kobject_free_release(obj);
                return 0;
        } else if (obj->type == KOBJ_TYPE_PORT_CLIENT_REF) {
                port_client_close(obj->ptr);
                kobject_free_release(obj);
                return 0;
        } else if (obj->type == KOBJ_TYPE_PORT_IFCE) {
                port_ifce_t *ifce = obj->ptr;
                ifce->close_func();
                kobject_free_release(obj);
                return 0;
        } else {
                mutex_release(&obj->lock);
                return -KERN_ERROR_INVAL;
        }
}

s64
syscall_port_request(kobject_handler_t handler, port_request_user_t *_req)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_t *obj = kobject_lock_fetch(CURRENT_PROCESS, handler);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!obj) { return -KERN_ERROR_INVAL; }

        if (obj->type != KOBJ_TYPE_PORT_CLIENT_REF
            && obj->type != KOBJ_TYPE_PORT_IFCE) {
                mutex_release(&obj->lock);
                return -KERN_ERROR_INVAL;
        }

        port_client_ref_t *ref = obj->ptr;

        port_request_user_t user_req;

        if (!user_memory_read(
                CURRENT_ADDRESS_SPACE, &user_req, _req,
                sizeof(port_request_user_t))) {
                /* TODO: kill process */
                PANIC("unimplemented process kill");
        }

        if (user_req.type < PORT_REQ_TYPE_CUSTOM_START) {
                mutex_release(&obj->lock);
                return -KERN_ERROR_INVAL;
        }

        if (obj->type == KOBJ_TYPE_PORT_IFCE) {
                /* invoke port-like interface */
                mutex_acquire(&CURRENT_ADDRESS_SPACE->lock);
                if (user_req.data_addr
                    && !user_memory_check_read(
                        CURRENT_ADDRESS_SPACE, user_req.data_addr,
                        user_req.data_size)) {
                        mutex_release(&CURRENT_ADDRESS_SPACE->lock);
                        PANIC("unimplemented process kill");
                }

                if (user_req.retval_addr
                    && !user_memory_check_write(
                        CURRENT_ADDRESS_SPACE, user_req.retval_addr,
                        user_req.retval_size)) {
                        mutex_release(&CURRENT_ADDRESS_SPACE->lock);
                        PANIC("unimplemented process kill");
                }
                mutex_release(&CURRENT_ADDRESS_SPACE->lock);

                port_ifce_t *ifce  = obj->ptr;
                user_req.val_small = ifce->req_func(
                    user_req.val_small, user_req.data_addr, user_req.data_size,
                    user_req.retval_addr, user_req.retval_size);
                if (!user_memory_write(
                        CURRENT_ADDRESS_SPACE, _req, &user_req,
                        sizeof(port_request_user_t))) {
                        PANIC("unimplemented process kill");
                }

                mutex_release(&obj->lock);
                return KERN_OK;
        }

        kmemset(&ref->req, 0, sizeof(port_request_t));

        ref->req.type                = user_req.type;
        ref->req.sender              = ref;
        ref->req.val_small           = user_req.val_small;
        ref->req.data_sender_vaddr   = user_req.data_addr;
        ref->req.data_size           = user_req.data_size;
        ref->req.retval_sender_vaddr = user_req.retval_addr;
        ref->req.retval_size         = user_req.retval_size;

        int error = KERN_OK;

        port_request(ref, &error);
        user_req.val_small = ref->req.val_small;

        mutex_release(&obj->lock);

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _req, &user_req,
                sizeof(port_request_user_t))) {
                /* TODO: kill process */
                PANIC("unimplmeneted process kill");
        }

        return -error;
}

s64
syscall_port_receive(
    kobject_handler_t ref_h, port_request_user_t *_req, void *buf,
    size_t buflen)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_handler_t request_h;
        kobject_t *       request_obj =
            kobject_alloc_lock(CURRENT_PROCESS, &request_h);
        if (!request_obj) {
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_NOMEM;
        }

        kobject_t *ref_obj = kobject_lock_fetch(CURRENT_PROCESS, ref_h);
        if (!ref_obj) {
                kobject_free_release(request_obj);
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_INVAL;
        }

        mutex_release(&CURRENT_PROCESS->lock);

        if (ref_obj->type != KOBJ_TYPE_PORT_SERVER_REF) {
                kobject_free_release(request_obj);
                mutex_release(&ref_obj->lock);
                return -KERN_ERROR_INVAL;
        }

        port_request_t *req = port_receive(ref_obj->ptr, buf, buflen);
        ASSERT(req);

        mutex_release(&ref_obj->lock);

        request_obj->type = KOBJ_TYPE_PORT_REQUEST;
        request_obj->ptr  = req;

        port_request_user_t user_req;
        user_req.sender_pid  = req->sender->holder->pid;
        user_req.type        = req->type;
        user_req.data_size   = req->data_size;
        user_req.retval_size = req->retval_size;

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _req, &user_req,
                sizeof(port_request_user_t))) {
                /* TODO: kill process */
                PANIC("unimplemented kill process");
        }

        mutex_release(&request_obj->lock);
        return request_h;
}

s64
syscall_port_response(
    kobject_handler_t req_handler, u64 retval_small, void *retval,
    size_t retval_size)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_t *obj = kobject_lock_fetch(CURRENT_PROCESS, req_handler);
        mutex_release(&CURRENT_PROCESS->lock);

        if (obj->type != KOBJ_TYPE_PORT_REQUEST) {
                mutex_release(&obj->lock);
                return -KERN_ERROR_INVAL;
        }

        port_request_t *req = obj->ptr;
        req->val_small      = retval_small;

        int error = KERN_OK;
        port_response(req, retval, retval_size, &error);

        if (error == KERN_OK) {
                kobject_free_release(obj);
                return KERN_OK;
        }

        return -error;
}

void __thread_spawn_start(void *user_entry);

s64
syscall_thread_spawn(u64 user_entry)
{
        mutex_acquire(&CURRENT_PROCESS->lock);
        thread_t *new = thread_create(CURRENT_PROCESS);
        mutex_release(&CURRENT_PROCESS->lock);

        new->state            = THREAD_STATE_READY;
        new->sched_data.class = CURRENT_THREAD->sched_data.class;
        thread_start(new, __thread_spawn_start, (void *)user_entry);
        sched_enter(new);
        return new->tid;
}

s64
syscall_process_spawn(kobject_handler_t as_handler, u64 user_entry)
{
        if (as_handler == 0) {
                /* can not use current address space */
                return -KERN_ERROR_INVAL;
        }
        mutex_acquire(&CURRENT_PROCESS->lock);
        process_t *new_proc = process_create(CURRENT_PROCESS);
        if (!new_proc) {
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_NOMEM;
        }

        /*
         * Here we clone thread first, then address space
         * because it costs much less to rollback thread creation
         */
        thread_t *new_t = thread_create(new_proc);

        mutex_release(&CURRENT_PROCESS->lock);
        if (!new_t) {
                process_destroy(new_proc);
                return -KERN_ERROR_NOMEM;
        }

        mutex_acquire(&CURRENT_PROCESS->lock);
        address_space_t *as = as_lock_fetch(as_handler);
        mutex_release(&CURRENT_PROCESS->lock);

        if (!as) {
                thread_destroy(new_t);
                process_destroy(new_proc);
                return -KERN_ERROR_INVAL;
        }

        new_proc->address_space = as;
        ++as->ref_count;
        mutex_release(&as->lock);

        new_t->state            = THREAD_STATE_READY;
        new_t->sched_data.class = SCHED_CLASS_NORMAL;
        thread_start(new_t, __thread_spawn_start, (void *)user_entry);
        sched_enter(new_t);
        return new_proc->pid;
}

s64
syscall_thread_exit(u64 retval)
{
        CURRENT_THREAD->retval = retval;
        thread_terminate(CURRENT_THREAD);

        return KERN_OK;
}

s64
syscall_process_exit(u64 retval)
{
        mutex_acquire(&CURRENT_PROCESS->lock);

        CURRENT_PROCESS->retval = retval;

        /* TODO: reschedule signal */
        LIST_FOREACH(CURRENT_PROCESS->thread_list, p)
        {
                thread_t *t = CONTAINER_OF(p, thread_t, proc_list_node);
                if (t == CURRENT_THREAD) continue;
                thread_terminate(t);
        }

        mutex_release(&CURRENT_PROCESS->lock);
        thread_terminate(CURRENT_THREAD);

        return KERN_OK;
}

static thread_t *
thread_wait_any(void)
{
        while (B_TRUE) {
                if (atomic_load_uint(
                        CURRENT_PROCESS->thread_count, __ATOMIC_ACQUIRE)
                    == 1) {
                        /* there's nobody but you */
                        return NULL;
                }

                if (CURRENT_PROCESS->running_thread_count
                    == CURRENT_PROCESS->thread_count) {
                        sched_resched();
                        continue;
                }

                mutex_acquire(&CURRENT_PROCESS->lock);
                LIST_FOREACH_MUT(CURRENT_PROCESS->thread_list, p, __next)
                {
                        thread_t *t = CONTAINER_OF(p, thread_t, proc_list_node);
                        if (t == CURRENT_THREAD) { continue; }

                        if (atomic_load_uint(t->wait_count, __ATOMIC_ACQUIRE)
                            != 0) {
                                /* already being waited */
                                continue;
                        }

                        if (t->state != THREAD_STATE_EXITED) { continue; }

                        list_remove(&t->proc_list_node);
                        mutex_release(&CURRENT_PROCESS->lock);
                        return t;
                }
                mutex_release(&CURRENT_PROCESS->lock);
                sched_resched();
        }
}

s64
syscall_thread_wait(tid_t tid, thread_state_t *_stat)
{
        thread_state_t stat;

        if (tid == (tid_t)-1) {
                thread_t *t    = thread_wait_any();
                stat.retval    = t->retval;
                stat.thread_id = t->tid;

                /* wait for thread to stop */
                while (t->running) {
                        /* TODO: proper blocking */
                        sched_resched();
                }

                thread_destroy(t);
        } else {
                thread_t *target = NULL;
                mutex_acquire(&CURRENT_PROCESS->lock);
                LIST_FOREACH(CURRENT_PROCESS->thread_list, p)
                {
                        thread_t *t =
                            CONTAINER_OF(p, thread_t, sched_list_node);
                        if (t->tid != tid) { continue; }

                        atomic_inc_fetch_uint(t->wait_count, __ATOMIC_RELAXED);
                        target = t;
                        break;
                }
                mutex_release(&CURRENT_PROCESS->lock);

                if (!target) {
                        /* not found */
                        return -KERN_ERROR_INVAL;
                }

                /* wait for thread to stop */
                while (target->running) {
                        /* TODO: proper blocking */
                        sched_resched();
                }

                stat.retval    = target->retval;
                stat.thread_id = target->tid;
                ASSERT(target->tid == tid);

                if (!atomic_dec_fetch_uint(
                        target->wait_count, __ATOMIC_ACQ_REL)) {
                        thread_destroy(target);
                }
        }

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _stat, &stat, sizeof(stat))) {
                mutex_acquire(&CURRENT_PROCESS->lock);
                process_terminate(CURRENT_PROCESS);
                mutex_release(&CURRENT_PROCESS->lock);
        }

        return KERN_OK;
}

static process_t *
process_wait_any(void)
{
        while (B_TRUE) {
                mutex_acquire(&CURRENT_PROCESS->lock);

                if (list_is_empty(&CURRENT_PROCESS->child_list_head)) {
                        /* don't have a child */
                        mutex_release(&CURRENT_PROCESS->lock);
                        return NULL;
                }

                /* TODO proper blocking */
                LIST_FOREACH(CURRENT_PROCESS->child_list_head, p)
                {
                        process_t *proc =
                            CONTAINER_OF(p, process_t, sibling_list_node);

                        if (atomic_load_uint(
                                proc->wait_count, __ATOMIC_ACQUIRE)) {
                                /* already begin waited, skip this one */
                                continue;
                        }

                        if (atomic_load_uint(
                                proc->running_thread_count, __ATOMIC_ACQUIRE)) {
                                /* still running */
                                continue;
                        }

                        list_remove(&proc->sibling_list_node);
                        mutex_release(&CURRENT_PROCESS->lock);
                        return proc;
                }

                mutex_release(&CURRENT_PROCESS->lock);
                sched_resched();
        }
}

s64
syscall_process_wait(pid_t pid, process_state_t *_stat)
{
        process_state_t stat;

        process_t *proc;
        if (pid == (pid_t)-1) {
                proc = process_wait_any();
        } else {
                mutex_acquire(&CURRENT_PROCESS->lock);
                proc = NULL;
                LIST_FOREACH(CURRENT_PROCESS->child_list_head, p)
                {
                        process_t *pp =
                            CONTAINER_OF(p, process_t, sibling_list_node);

                        if (pp->pid != pid) { continue; }

                        proc = pp;
                }
		mutex_release(&CURRENT_PROCESS->lock);

                if (!proc) { return -KERN_ERROR_INVAL; }
        }

        /* wait for every thread to exit */
        LIST_FOREACH_MUT(proc->thread_list, p, __next)
        {
                thread_t *t = CONTAINER_OF(p, thread_t, proc_list_node);
                while (t->running) { sched_resched(); }

                thread_destroy(t);
        }

        ASSERT(proc->thread_count == 0);

        stat.process_id = proc->pid;
        stat.retval     = proc->retval;

        process_destroy(proc);

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _stat, &stat, sizeof(stat))) {
                mutex_acquire(&CURRENT_PROCESS->lock);
                process_terminate(CURRENT_PROCESS);
                mutex_release(&CURRENT_PROCESS->lock);
        }

        return KERN_OK;
}

void NORETURN __reincarnate_return(u64 user_entry);

s64
syscall_reincarnate(kobject_handler_t as_h, u64 user_entry)
{
        boolean flag = atomic_xchg_boolean(
            CURRENT_PROCESS->reincarnation_flag, B_TRUE, __ATOMIC_ACQ_REL);
        if (flag) {
                /* there's already an ongoing reincarnation */
                thread_terminate(CURRENT_THREAD);
                sched_resched();
        }

        mutex_acquire(&CURRENT_PROCESS->lock);

        address_space_t *as = as_lock_fetch(as_h);
        if (!as) {
                mutex_release(&CURRENT_PROCESS->lock);
                return -KERN_ERROR_INVAL;
        }
        mutex_release(&as->lock);

        LIST_FOREACH_MUT(CURRENT_PROCESS->thread_list, p, __next)
        {
                thread_t *t = CONTAINER_OF(p, thread_t, proc_list_node);
                if (t == CURRENT_THREAD) continue;

                thread_terminate(t);

                while (t->running) {
                        /* TODO: proper blocking */
                        sched_resched();
                }

                thread_destroy(t);
        }

        address_space_t *old_as        = CURRENT_PROCESS->address_space;
        CURRENT_PROCESS->address_space = as;

        mutex_acquire_dual(&old_as->lock, &as->lock);
        ++as->ref_count;
        uint rc = --old_as->ref_count;
        mutex_release_dual(&old_as->lock, &as->lock);

        if (!rc) { vm_address_space_destroy(old_as); }

        vm_address_space_load(as);

        mutex_release(&CURRENT_PROCESS->lock);

        __reincarnate_return(user_entry);
        PANIC("should not reach further than __reincarnation_return");
}

s64
syscall_futex_wait(void *addr, futex_val_t val)
{
        futex_wait(CURRENT_ADDRESS_SPACE, addr, val);
        return KERN_OK;
}

s64
syscall_futex_wake(void *addr, size_t count)
{
        futex_wake(CURRENT_ADDRESS_SPACE, addr, count);
        return KERN_OK;
}

void *__syscall_table[__SYSCALL_COUNT];
uint  __syscall_count       = __SYSCALL_COUNT;
uint  __syscall_invil_error = -KERN_ERROR_INVAL;

extern void __syscall_entry(void);

void
syscall_def(void)
{
        /* initialize syscall table */
#define SYSCALLDEF(__no, __func) __syscall_table[__no] = __func
        SYSCALLDEF(SYSCALL_AS_CREATE, syscall_as_create);
        SYSCALLDEF(SYSCALL_AS_CLONE, syscall_as_clone);
        SYSCALLDEF(SYSCALL_AS_DESTROY, syscall_as_destroy);
        SYSCALLDEF(SYSCALL_MMAP, syscall_mmap);
        SYSCALLDEF(SYSCALL_MTRANSFER, syscall_mtransfer);
        SYSCALLDEF(SYSCALL_MUNMAP, syscall_munmap);
        SYSCALLDEF(SYSCALL_PORT_CREATE, syscall_port_create);
        SYSCALLDEF(SYSCALL_PORT_OPEN, syscall_port_open);
        SYSCALLDEF(SYSCALL_PORT_CLOSE, syscall_port_close);
        SYSCALLDEF(SYSCALL_PORT_REQUEST, syscall_port_request);
        SYSCALLDEF(SYSCALL_PORT_RECEIVE, syscall_port_receive);
        SYSCALLDEF(SYSCALL_PORT_RESPONSE, syscall_port_response);
        SYSCALLDEF(SYSCALL_THREAD_SPAWN, syscall_thread_spawn);
        SYSCALLDEF(SYSCALL_PROCESS_SPAWN, syscall_process_spawn);
        SYSCALLDEF(SYSCALL_THREAD_EXIT, syscall_thread_exit);
        SYSCALLDEF(SYSCALL_PROCESS_EXIT, syscall_process_exit);
        SYSCALLDEF(SYSCALL_THREAD_WAIT, syscall_thread_wait);
        SYSCALLDEF(SYSCALL_PROCESS_WAIT, syscall_process_wait);
        SYSCALLDEF(SYSCALL_REINCARNATE, syscall_reincarnate);
        SYSCALLDEF(SYSCALL_FUTEX_WAIT, syscall_futex_wait);
        SYSCALLDEF(SYSCALL_FUTEX_WAKE, syscall_futex_wake);
#undef SYSCALLDEF
}

void
syscall_init(void)
{
        /* enable EFER.SCE */
        wrmsr(0xc0000080, rdmsr(0xc0000080) | 1);

        /* STAR */
        wrmsr(0xc0000081, (0x08ul << 32) | (0x10ul << 48));

        /* LSTAR */
        wrmsr(0xc0000082, (u64)__syscall_entry);

        /* SFMASK */
        wrmsr(0xc0000084, 1 << 9);
}
