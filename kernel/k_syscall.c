/* k_syscall.c -- Syscall implementation */

/*
 * Copyright 2021 Mosakuji Hokuto <shikieiki@yamaxanadu.org>.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <hal_percpu.h>
#include <k_asm.h>
#include <k_cdefs.h>
#include <k_console.h>
#include <k_futex.h>
#include <k_memory.h>
#include <k_port.h>
#include <k_port_ifce.h>
#include <k_string.h>
#include <k_syscall.h>
#include <k_user_mem.h>

#include <osrt/error.h>
#include <osrt/syscall.h>

s64
syscall_as_create(void)
{
        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_handler_t h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_THREAD, &h);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) { return -ERROR(NOMEM); }

        address_space_t *as = vm_address_space_create();
        if (!as) {
                kobject_free_release(obj);
                return -ERROR(NOMEM);
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

        kobject_t *obj = kobject_lock_fetch(CURRENT_THREAD, h);

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
        mutex_acquire(&CURRENT_THREAD->lock);
        address_space_t *src = as_lock_fetch(src_h);
        if (!src) {
                mutex_release(&CURRENT_THREAD->lock);
                return -ERROR(INVAL);
        }

        kobject_handler_t ret_h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_THREAD, &ret_h);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) {
                mutex_release(&src->lock);
                return -ERROR(NOMEM);
        }

        address_space_t *cloned = vm_address_space_clone(src);
        if (!cloned) {
                mutex_release(&src->lock);
                kobject_free_release(obj);
                return -ERROR(NOMEM);
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
                return -ERROR(INVAL);
        }

        mutex_acquire(&CURRENT_THREAD->lock);
        address_space_t *as = as_lock_fetch(h);
        mutex_release(&CURRENT_THREAD->lock);

        if (!as) { return -ERROR(INVAL); }

        if (!(--as->ref_count)) {
                mutex_acquire(&CURRENT_THREAD->lock);

                kobject_t *obj = kobject_lock_fetch(CURRENT_THREAD, h);

                mutex_release(&as->lock);
                vm_address_space_destroy(as);

                kobject_free_release(obj);

                mutex_release(&CURRENT_THREAD->lock);
        } else {
                mutex_release(&as->lock);
        }

        return OK;
}

s64
syscall_mmap(
    kobject_handler_t as_handler, uintptr vaddr, size_t size, uint flag)
{
        if ((vaddr & (1ull << 47)) != 0) {
                /* can not map kernel space */
                return -ERROR(DENIED);
        }

        if (((vaddr + size) & (1ull << 47)) != 0) {
                /* can not map kernel space */
                return -ERROR(DENIED);
        }

        if (flag & ~USER_ALLOWED_FLAGS) { return -ERROR(DENIED); }

        mutex_acquire(&CURRENT_THREAD->lock);
        address_space_t *as = as_lock_fetch(as_handler);
        mutex_release(&CURRENT_THREAD->lock);

        if (!vaddr)
                vaddr = (uintptr)vm_alloc_vaddr(
                    as, USERLAND_VADDR_BEGIN, USERLAND_VADDR_END, size);
        if (!vaddr) {
                mutex_release(&as->lock);
                return -ERROR(NOMEM);
        }

        if (VM_OFFSET_PAGE_2M(vaddr) != 0) {
                mutex_release(&as->lock);
                return -ERROR(INVAL);
        }

        uintptr vi = vaddr;
        for (size_t mapped = 0; mapped < size; mapped += KMEM_PAGE_SIZE) {
                uintptr paddr = pma_alloc(PMA_ZONE_ANY);
                if (!paddr) {
                        vm_unmap_n(as, (void *)vaddr, mapped);
                        mutex_release(&as->lock);
                        return -ERROR(NOMEM);
                }

                if (vm_is_mapped(as, (void *)vi)) {
                        vm_unmap_n(as, (void *)vaddr, mapped);
                        mutex_release(&as->lock);
                        return -ERROR(INVAL);
                }

                if (!vm_map_page(as, (void *)vi, paddr, flag)) {
                        vm_unmap_n(as, (void *)vi, mapped);
                        mutex_release(&as->lock);
                        return -ERROR(NOMEM);
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
        if (!vaddr_s && !vaddr_d) { return -ERROR(INVAL); }

        if (vaddr_s >= (uintptr)USERLAND_VADDR_END) { return -ERROR(DENIED); }

        if (vaddr_d >= (uintptr)USERLAND_VADDR_END) { return -ERROR(DENIED); }

        if ((vaddr_s + size) > (uintptr)USERLAND_VADDR_END) {
                return -ERROR(DENIED);
        }

        if ((vaddr_d + size) > (uintptr)USERLAND_VADDR_END) {
                return -ERROR(DENIED);
        }

        if (vaddr_s < (uintptr)USERLAND_VADDR_BEGIN) { return -ERROR(INVAL); }

        if (vaddr_d < (uintptr)USERLAND_VADDR_BEGIN) { return -ERROR(INVAL); }

        if (VM_OFFSET_PAGE_2M(vaddr_s) != 0) { return -ERROR(INVAL); }

        if (VM_OFFSET_PAGE_2M(vaddr_d) != 0) { return -ERROR(INVAL); }

        if (flag & ~USER_ALLOWED_FLAGS) { return -ERROR(DENIED); }

        mutex_acquire(&CURRENT_THREAD->lock);
        address_space_t *src = as_lock_fetch(sh);
        if (!src) {
                mutex_release(&CURRENT_THREAD->lock);
                return -ERROR(INVAL);
        }

        address_space_t *dst = as_lock_fetch(dh);
        if (!dst) {
                mutex_release(&src->lock);
                mutex_release(&CURRENT_THREAD->lock);
                return -ERROR(INVAL);
        }

        if (!vaddr_d) {
                /* allocate virtual address for dst */
                vaddr_d = (uintptr)vm_alloc_vaddr(
                    dst, USERLAND_VADDR_BEGIN, USERLAND_VADDR_END, size);
                if (!vaddr_d) {
                        mutex_release(&dst->lock);
                        mutex_release(&src->lock);
                        mutex_release(&CURRENT_THREAD->lock);
                        return -ERROR(NOMEM);
                }
        }
        mutex_release(&CURRENT_THREAD->lock);

        uintptr si = vaddr_s;
        uintptr di = vaddr_d;
        for (size_t mapped = 0; mapped < size; mapped += KMEM_PAGE_SIZE) {
                if (!vm_is_mapped(src, (void *)si)) {
                        vm_unmap_n(dst, (void *)vaddr_d, mapped);
                        mutex_release(&src->lock);
                        mutex_release(&dst->lock);
                        return -ERROR(INVAL);
                }

                if (vm_is_mapped(dst, (void *)di)) {
                        vm_unmap_n(dst, (void *)vaddr_d, mapped);
                        mutex_release(&src->lock);
                        mutex_release(&dst->lock);
                        return -ERROR(INVAL);
                }

                uintptr pma = vm_get_pma_by_vma(
                    vm_get_vma_for_pma(src->pml4_paddr), (void *)si);
                pma_inc_ref_count(pma, KMEM_PAGE_SIZE);
                void *map_ret = vm_map_page(dst, (void *)di, pma, flag);
                if (!map_ret) {
                        vm_unmap_n(dst, (void *)vaddr_d, mapped);
                        mutex_release(&src->lock);
                        mutex_release(&dst->lock);
                        return -ERROR(NOMEM);
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
        if ((vaddr & (1ull << 47)) != 0) { return -ERROR(DENIED); }

        if (((vaddr + size) & (1ull << 47)) != 0) {
                /* can not unmap kernel space */
                return -ERROR(DENIED);
        }

        address_space_t *as = as_lock_fetch(h);
        if (!as) { return -ERROR(INVAL); }

        for (size_t vi = vaddr; vi < (vaddr + size); vi += KMEM_PAGE_SIZE) {
                if (!vm_is_mapped(as, (void *)vi)) {
                        mutex_release(&as->lock);
                        return -ERROR(DENIED);
                }
        }

        vm_unmap_n(as, (void *)vaddr, size);
        mutex_release(&as->lock);
        return 0;
}

s64
syscall_port_create(char *name, size_t namelen)
{
        if (namelen >= PORT_NAME_MAX_LEN) { return -ERROR(INVAL); }

        char namebuf[PORT_NAME_MAX_LEN];
        if (!user_memory_read(CURRENT_ADDRESS_SPACE, namebuf, name, namelen)) {
                thread_raise_exception(
                    CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
        }
        namebuf[namelen] = '\0';

        mutex_acquire(&CURRENT_THREAD->lock);
        int               error = OK;
        kobject_handler_t h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_THREAD, &h);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) { return -ERROR(NOMEM); }

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
        if (namelen >= PORT_NAME_MAX_LEN) { return -ERROR(INVAL); }

        char namebuf[PORT_NAME_MAX_LEN];
        if (!user_memory_read(CURRENT_ADDRESS_SPACE, namebuf, name, namelen)) {
                thread_raise_exception(
                    CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
        }
        namebuf[namelen] = '\0';

        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_handler_t h;
        kobject_t *       obj = kobject_alloc_lock(CURRENT_THREAD, &h);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) { return -ERROR(NOMEM); }

        /* check if we have a port-like interface */
        port_ifce_t *ifce = port_ifce_lookup(namebuf);
        if (ifce) {
                sint error = ifce->open_func();
                if (error != OK) {
                        kobject_free_release(obj);
                        return -error;
                }

                obj->type = KOBJ_TYPE_PORT_IFCE;
                obj->ptr  = ifce;

                mutex_release(&obj->lock);
                return h;
        }

        /* normal port open */
        int                error = OK;
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
        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_t *obj = kobject_lock_fetch(CURRENT_THREAD, handler);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) { return -ERROR(INVAL); }

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
                return -ERROR(INVAL);
        }
}

s64
syscall_port_request(kobject_handler_t handler, port_request_user_t *_req)
{
        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_t *obj = kobject_lock_fetch(CURRENT_THREAD, handler);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) { return -ERROR(INVAL); }

        if (obj->type != KOBJ_TYPE_PORT_CLIENT_REF
            && obj->type != KOBJ_TYPE_PORT_IFCE) {
                mutex_release(&obj->lock);
                return -ERROR(INVAL);
        }

        port_client_ref_t *ref = obj->ptr;

        port_request_user_t user_req;

        if (!user_memory_read(
                CURRENT_ADDRESS_SPACE, &user_req, _req,
                sizeof(port_request_user_t))) {
                thread_raise_exception(
                    CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
        }

        if (obj->type == KOBJ_TYPE_PORT_IFCE) {
                /* invoke port-like interface */
                mutex_acquire(&CURRENT_ADDRESS_SPACE->lock);
                if (user_req.data_addr
                    && !user_memory_check_read(
                        CURRENT_ADDRESS_SPACE, user_req.data_addr,
                        user_req.data_size)) {
                        mutex_release(&CURRENT_ADDRESS_SPACE->lock);
                        thread_raise_exception(
                            CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
                }

                if (user_req.retval_addr
                    && !user_memory_check_write(
                        CURRENT_ADDRESS_SPACE, user_req.retval_addr,
                        user_req.retval_size)) {
                        mutex_release(&CURRENT_ADDRESS_SPACE->lock);
                        thread_raise_exception(
                            CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
                }
                mutex_release(&CURRENT_ADDRESS_SPACE->lock);

                port_ifce_t *ifce  = obj->ptr;
                user_req.val_small = ifce->req_func(
                    user_req.val_small, user_req.data_addr, user_req.data_size,
                    user_req.retval_addr, user_req.retval_size);
                if (!user_memory_write(
                        CURRENT_ADDRESS_SPACE, _req, &user_req,
                        sizeof(port_request_user_t))) {
                        thread_raise_exception(
                            CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
                }

                mutex_release(&obj->lock);
                return OK;
        }

        kmemset(&ref->req, 0, sizeof(port_request_t));

        ref->req.sender              = ref;
        ref->req.val_small           = user_req.val_small;
        ref->req.data_sender_vaddr   = user_req.data_addr;
        ref->req.data_size           = user_req.data_size;
        ref->req.retval_sender_vaddr = user_req.retval_addr;
        ref->req.retval_size         = user_req.retval_size;

        int error = OK;

        port_request(ref, &error);
        user_req.val_small = ref->req.val_small;

        mutex_release(&obj->lock);

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _req, &user_req,
                sizeof(port_request_user_t))) {
                thread_raise_exception(
                    CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
        }

        return -error;
}

s64
syscall_port_receive(
    kobject_handler_t ref_h, port_request_user_t *_req, void *buf,
    size_t buflen)
{
        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_t *ref_obj = kobject_lock_fetch(CURRENT_THREAD, ref_h);
        if (!ref_obj) {
                mutex_release(&CURRENT_THREAD->lock);
                return -ERROR(INVAL);
        }
        mutex_release(&CURRENT_THREAD->lock);

        if (ref_obj->type != KOBJ_TYPE_PORT_SERVER_REF) {
                mutex_release(&ref_obj->lock);
                return -ERROR(INVAL);
        }

        port_request_t *req = port_receive(ref_obj->ptr, buf, buflen);
        ASSERT(req);

        port_request_user_t user_req;
        user_req.val_small   = req->val_small;
        user_req.sender_pid  = req->sender->holder->tid;
        user_req.data_size   = req->data_size;
        user_req.retval_size = req->retval_size;

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _req, &user_req,
                sizeof(port_request_user_t))) {
                thread_raise_exception(
                    CURRENT_THREAD, THREAD_EXCEPTION_ACCESS_VIOLATION);
        }

        mutex_release(&ref_obj->lock);

        return OK;
}

s64
syscall_port_response(
    kobject_handler_t ref_handler, u64 retval_small, void *retval,
    size_t retval_size)
{
        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_t *obj = kobject_lock_fetch(CURRENT_THREAD, ref_handler);
        mutex_release(&CURRENT_THREAD->lock);

        if (!obj) { return -ERROR(INVAL); }

        if (obj->type != KOBJ_TYPE_PORT_SERVER_REF) {
                mutex_release(&obj->lock);
                return -ERROR(INVAL);
        }

        port_server_ref_t *ref = obj->ptr;

        int error = OK;
        port_response(ref, retval_small, retval, retval_size, &error);
        mutex_release(&obj->lock);
        return -error;
}

void __thread_spawn_start(void *user_entry);

s64
syscall_thread_spawn(kobject_handler_t as_handler, u64 user_entry)
{
        if (as_handler == 0) {
                /* can not use current address space */
                return -ERROR(INVAL);
        }
        mutex_acquire(&CURRENT_THREAD->lock);
        thread_t *new_thread = thread_create(NULL, CURRENT_THREAD);
        if (!new_thread) {
                mutex_release(&CURRENT_THREAD->lock);
                return -ERROR(NOMEM);
        }

        address_space_t *as = as_lock_fetch(as_handler);

        if (!as) {
                thread_destroy(new_thread);
                return -ERROR(INVAL);
        }

        mutex_release(&CURRENT_THREAD->lock);

        new_thread->address_space = as;
        ++as->ref_count;
        mutex_release(&as->lock);

        new_thread->state          = THREAD_STATE_READY;
        new_thread->sched_data.class = SCHED_CLASS_NORMAL;
        thread_start(new_thread, __thread_spawn_start, (void *)user_entry);
        sched_enter(new_thread);
        return new_thread->tid;
}

s64
syscall_thread_exit(u64 retval)
{
        mutex_acquire(&CURRENT_THREAD->lock);
        CURRENT_THREAD->retval = retval;
        mutex_release(&CURRENT_THREAD->lock);

        thread_terminate(CURRENT_THREAD);

        /* will never return to userland */
        return OK;
}

s64
syscall_thread_wait(tid_t tid, thread_state_t *_stat)
{
        thread_state_t stat;

        thread_t *th = NULL;
        if (tid == (tid_t)-1) {
                while (!th) {
                        futex_val_t exited_child =
                            CURRENT_THREAD->exited_child_count;

                        mutex_acquire(&CURRENT_THREAD->lock);
                        LIST_FOREACH(CURRENT_THREAD->child_list_head, p)
                        {
                                thread_t *pp = CONTAINER_OF(
                                    p, thread_t, sibling_list_node);
                                if (!atomic_load_boolean(
                                        pp->terminate_flag, __ATOMIC_ACQUIRE)) {
                                        continue;
                                }
                                th = pp;
                        }
                        mutex_release(&CURRENT_THREAD->lock);

                        /* wait until there's child exit */
                        futex_kwait(
                            &CURRENT_THREAD->exited_child_count, exited_child);
                }
        } else {
                mutex_acquire(&CURRENT_THREAD->lock);
                LIST_FOREACH(CURRENT_THREAD->child_list_head, p)
                {
                        thread_t *pp =
                            CONTAINER_OF(p, thread_t, sibling_list_node);

                        if (pp->tid != tid) { continue; }

                        th = pp;
                }
                mutex_release(&CURRENT_THREAD->lock);

                if (!th) { return -ERROR(INVAL); }
        }

        /* wait for process to exit */
        while (B_TRUE) {
                u64 state = th->state;
                if (state == THREAD_STATE_EXITED) break;
                futex_kwait(&th->state, state);
        }

        stat.thread_id = th->tid;
        stat.retval     = th->retval;

        thread_destroy(th);

        if (!user_memory_write(
                CURRENT_ADDRESS_SPACE, _stat, &stat, sizeof(stat))) {
                mutex_acquire(&CURRENT_THREAD->lock);
                thread_terminate(CURRENT_THREAD);
                mutex_release(&CURRENT_THREAD->lock);
        }

        return OK;
}

void NORETURN __reincarnate_return(u64 user_entry);

s64
syscall_reincarnate(kobject_handler_t as_h, u64 user_entry)
{
        mutex_acquire(&CURRENT_THREAD->lock);

        address_space_t *as = as_lock_fetch(as_h);
        if (!as) {
                mutex_release(&CURRENT_THREAD->lock);
                return -ERROR(INVAL);
        }

        /* unlock and relock to prevent deadlock */
        mutex_release(&as->lock);

        address_space_t *old_as        = CURRENT_THREAD->address_space;
        CURRENT_THREAD->address_space = as;

        mutex_acquire_dual(&old_as->lock, &as->lock);
        ++as->ref_count;
        uint rc = --old_as->ref_count;
        mutex_release_dual(&old_as->lock, &as->lock);

        if (!rc) { vm_address_space_destroy(old_as); }

        vm_address_space_load(as);

        mutex_release(&CURRENT_THREAD->lock);

        __reincarnate_return(user_entry);
        PANIC("should not reach further than __reincarnation_return");
}

s64
syscall_futex_wait(void *addr, futex_val_t val)
{
        futex_wait(CURRENT_ADDRESS_SPACE, addr, val);
        return OK;
}

s64
syscall_futex_wake(void *addr, size_t count)
{
        futex_wake(CURRENT_ADDRESS_SPACE, addr, count);
        return OK;
}

void *__syscall_table[__OSRT_SYSCALL_COUNT];
uint  __syscall_count       = __OSRT_SYSCALL_COUNT;
uint  __syscall_invil_error = -ERROR(INVAL);

extern void __syscall_entry(void);

void
syscall_def(void)
{
        /* initialize syscall table */
#define SYSCALLDEF(__no, __func) __syscall_table[__OSRT_##__no] = __func
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
        SYSCALLDEF(SYSCALL_THREAD_EXIT, syscall_thread_exit);
        SYSCALLDEF(SYSCALL_THREAD_WAIT, syscall_thread_wait);
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
