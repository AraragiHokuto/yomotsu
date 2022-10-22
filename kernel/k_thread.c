/* k_proc.c -- thread implementation */

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
#include <k_kobj.h>
#include <k_memory.h>
#include <k_mutex.h>
#include <k_thread.h>
#include <k_simd.h>
#include <k_string.h>

#include <osrt/error.h>

#define KERNEL_STACK_SIZE 16384 /* 16KB */

extern void __thread_do_switch(
    void *dst_rsp, void *dst_rip, void *src_rsp, void *src_rip,
    thread_t *current);
extern void __thread_do_start(
    void *rsp, void *rip, void *entry, void *data, thread_t *current);

thread_t *__kernel_proc;

static void __thread_on_terminate(void);

void
__thread_save_context(thread_t *target)
{
        thread_t *current = CURRENT_THREAD;

        ASSERT(current != target);

        /* save FSBase */
        current->cpu_state.fsbase = rdmsr(0xc0000100);

        __thread_do_switch(
            &current->cpu_state.rsp, &current->cpu_state.rip,
            target->cpu_state.rsp, target->cpu_state.rip, current);
}

void
__thread_load_context(thread_t *target)
{
        thread_t *prev = CURRENT_THREAD;
        CURRENT_THREAD = target;

        ASSERT(prev != target);

        if (prev && prev->terminate_flag) {
                prev->state = THREAD_STATE_EXITED;

                /* wake parent in case it's being waited */
                futex_kwake(&prev->state, 1);
        }

        percpu()->tss->rsp0            = (u64)target->kernel_stack;
        percpu()->current_kernel_stack = target->kernel_stack;

        vm_address_space_load(target->address_space);

        /* load userland FSBase value */
        wrmsr(0xc0000100, target->cpu_state.fsbase);

        interrupt_load_flag(target->cpu_state.interrupt_flag);

        sched_enable();
}

void
__thread_on_sysret(void)
{
        if (atomic_load_boolean(
                CURRENT_THREAD->terminate_flag, __ATOMIC_ACQUIRE)) {
                __thread_on_terminate();
        }
}

void
__thread_on_user_iret(void)
{
        if (atomic_load_boolean(
                CURRENT_THREAD->terminate_flag, __ATOMIC_ACQUIRE)) {
                __thread_on_terminate();
        }
}

void
thread_switch_context(thread_t *target)
{
        __thread_save_context(target);
}

void
thread_start(thread_t *target, void *entry, void *data)
{
        target->cpu_state.rsp = target->kernel_stack;
        __thread_do_start(
            &target->cpu_state.rsp, &target->cpu_state.rip, entry, data,
            target);
}

#define TID_MAX            32768
#define TID_BITMAP_ENTRIES 32768
#define TID_BITMAP_SIZE    (TID_BITMAP_ENTRIES / 64)

static u64     tid_bitmap[TID_BITMAP_SIZE];
static mutex_t tid_bitmap_lock;

static void
tid_bitmap_set(size_t pos)
{
        ASSERT(pos < TID_BITMAP_ENTRIES);

        size_t idx = pos / 8;
        size_t off = pos % 8;

        ASSERT((tid_bitmap[idx] & (1ull << off)) == 0);

        tid_bitmap[idx] |= (1ull << off);
}

static void
tid_bitmap_clear(size_t pos)
{
        ASSERT(pos < TID_BITMAP_ENTRIES);

        size_t idx = pos / 8;
        size_t off = pos % 8;

        ASSERT((tid_bitmap[idx] & (1ull << off)) != 0);

        tid_bitmap[idx] &= ~(1ull << off);
}

static tid_t
alloc_tid(void)
{
        /* TODO: lockfree */
        mutex_acquire(&tid_bitmap_lock);

        size_t idx;
        size_t off;
        for (idx = 0; idx < TID_BITMAP_SIZE; ++idx) {
                if (tid_bitmap[idx] == ~(0ull)) { continue; }

                for (off = 0; off < 64; ++off) {
                        if (tid_bitmap[idx] & (1ull << off)) { continue; }

                        tid_t ret = idx * 8 + off;
                        tid_bitmap_set(ret);
                        mutex_release(&tid_bitmap_lock);
                        return ret + 1;
                }

                ASSERT(off < 64);
        }

        mutex_release(&tid_bitmap_lock);
        return -1;
}

static void
free_tid(tid_t tid)
{
        mutex_acquire(&tid_bitmap_lock);
        tid_bitmap_clear(tid);
        mutex_release(&tid_bitmap_lock);
}

void
thread_init(void)
{
        kmemset(tid_bitmap, 0, sizeof(tid_bitmap));
        mutex_init(&tid_bitmap_lock);

        tss_t *tss = kmem_alloc(sizeof(tss_t));
        kmemset(tss, 0, sizeof(tss_t));

        descriptor_load_tss(tss);
        percpu()->tss = tss;

        /* check fsgsbase support */
        u32 eax, ebx, ecx, edx;
        cpuid(0x07, &eax, &ebx, &ecx, &edx);
        if (ebx & 0x1) {
                u64 cr4;
                asm volatile("movq %%cr4, %0" : "=r"(cr4));
                cr4 |= (1 << 16);
                asm volatile("movq %0, %%cr4" ::"r"(cr4));
        } else {
                kprintf("FSGSBASE not supported\n");
        }

        __kernel_proc = thread_create(NULL, NULL);
        VERIFY(__kernel_proc, "failed to allocate kernel thread");

        __kernel_proc->address_space = vm_address_space_create();
        VERIFY(
            __kernel_proc->address_space,
            "failed to allocate address space for kernel proc");
}

/* assume holding parent's lock */
thread_t*
thread_create(thread_t *t, thread_t *parent)
{
        (void)t;
        tid_t tid = alloc_tid();
        if (!tid) return NULL;

        thread_t *ret = kmem_alloc(sizeof(thread_t));
        if (!ret) {
                free_tid(tid);
                return NULL;
        }

        byte *kernel_stack = kmem_alloc(KERNEL_STACK_SIZE);
        if (!kernel_stack) {
                kmem_free(ret);
                free_tid(tid);
                return NULL;
        }

        byte *simd_state = simd_alloc_state_area();
        if (!simd_state) {
                kmem_free(kernel_stack);
                kmem_free(ret);
                return NULL;
        }

        int kobject_error = OK;
        if (!kobject_init(ret, &kobject_error)) {
                simd_free_state_area(simd_state);
                kmem_free(kernel_stack);
                kmem_free(ret);
                free_tid(tid);
                return NULL;
        }

        byte *kernel_rsp   = kernel_stack + KERNEL_STACK_SIZE - 8;
        *(u64 *)kernel_rsp = 0;

        mutex_init(&ret->lock);

        ret->tid    = tid;
        ret->retval = 0;

        ret->terminate_flag = B_FALSE;

        list_head_init(&ret->sched_list_node);

        ret->cpu_state.simd_state = simd_state;

        ret->kernel_stack = kernel_rsp;

        ret->address_space = NULL;

        list_head_init(&ret->sibling_list_node);
        list_head_init(&ret->child_list_head);
        ret->exited_child_count = 0;

        if (parent) {
                ret->parent = parent;
                list_insert(&ret->sibling_list_node, &parent->child_list_head);
        }

        return ret;
}

/* assume holding parent's lock */
void
thread_destroy(thread_t *thread)
{
        ASSERT(list_is_empty(&thread->child_list_head));

        ASSERT(thread->parent);
        list_remove(&thread->sibling_list_node);

        if (thread->address_space
            && !atomic_dec_fetch_uint(
                thread->address_space->ref_count, __ATOMIC_ACQ_REL)) {
                vm_address_space_destroy(thread->address_space);
        }

        kmem_free((byte *)thread->kernel_stack - KERNEL_STACK_SIZE + 8);
        simd_free_state_area(thread->cpu_state.simd_state);

        free_tid(thread->tid);
        kmem_free(thread);
}

/* Assume holding thread lock */
void
thread_terminate(thread_t *thread)
{
        atomic_store_boolean(thread->terminate_flag, B_TRUE, __ATOMIC_RELEASE);
}

void
thread_raise_exception(thread_t *thread, int exception)
{
        /* stub */
        kprintf("thread killed due to exception: %d\n", exception);
        thread_terminate(thread);
}

static void
__thread_on_terminate(void)
{
        if (!CURRENT_THREAD->parent) {
                PANIC("thread with NULL parent terminated");
        }

        sched_disable();

        mutex_acquire(&CURRENT_THREAD->lock);
        kobject_cleanup(CURRENT_THREAD);
        mutex_acquire(&CURRENT_THREAD->parent->lock);

        LIST_FOREACH_MUT(CURRENT_THREAD->child_list_head, p, __next)
        {
                thread_t *child =
                    CONTAINER_OF(p, thread_t, sibling_list_node);
                mutex_try_acquire(&child->lock);
                child->parent = CURRENT_THREAD->parent;
                list_remove(&child->sibling_list_node);
                list_insert(
                    &child->sibling_list_node,
                    &CURRENT_THREAD->parent->child_list_head);
                mutex_release(&child->lock);
        }

        ++CURRENT_THREAD->parent->exited_child_count;
        futex_kwake(&CURRENT_THREAD->parent->exited_child_count, 1);

        mutex_release(&CURRENT_THREAD->parent->lock);

        mutex_release(&CURRENT_THREAD->lock);

        sched_leave(CURRENT_THREAD);
        sched_enable();
        sched_resched();

        PANIC("THREAD_ON_TERMINATE reached end");
}
