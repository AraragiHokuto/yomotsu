#include <kern/asm.h>
#include <kern/console.h>
#include <kern/error.h>
#include <kern/kobject.h>
#include <kern/macrodef.h>
#include <kern/memory.h>
#include <kern/mutex.h>
#include <kern/percpu.h>
#include <kern/process.h>
#include <kern/simd.h>
#include <kern/string.h>

#define KERNEL_STACK_SIZE 16384 /* 16KB */

extern void __thread_do_switch(
    void *dst_rsp, void *dst_rip, void *src_rsp, void *src_rip,
    thread_t *target);
extern void __thread_do_start(
    void *rsp, void *rip, void *entry, void *data, thread_t *target);

process_t *__kernel_proc;

static void __thread_on_terminate(void);
static void __process_on_terminate(void);

thread_t *
thread_create(process_t *container)
{
        thread_t *self = kmem_alloc(sizeof(thread_t));
        if (!self) return NULL;

        byte *kernel_stack = kmem_alloc(KERNEL_STACK_SIZE);
        if (!kernel_stack) {
                kmem_free(self);
                return NULL;
        };

        byte *simd_state = simd_alloc_state_area();
        if (!simd_state) {
                kmem_free(kernel_stack);
                kmem_free(self);
                return NULL;
        }

        byte *kernel_rsp   = kernel_stack + KERNEL_STACK_SIZE - 8;
        *(u64 *)kernel_rsp = 0;

        mutex_init(&self->lock);

        self->kernel_stack = kernel_rsp;
        self->container    = container;

        self->retval         = 0;
        self->terminate_flag = B_FALSE;
        self->running        = B_TRUE;

        self->wait_count = 0;

        self->cpu_state.simd_state = simd_state;

        self->tid = ++container->next_tid;
        list_insert(&self->proc_list_node, &container->thread_list);
        ++container->thread_count;
        ++container->running_thread_count;

        return self;
}

void
thread_destroy(thread_t *thread)
{
        list_remove(&thread->proc_list_node);

        atomic_dec_fetch_uint(
            thread->container->thread_count, __ATOMIC_RELAXED);

        simd_free_state_area(thread->cpu_state.simd_state);
        kmem_free((byte *)thread->kernel_stack - KERNEL_STACK_SIZE + 8);
        kmem_free(thread);
}

void
__thread_save_context(thread_t *target)
{
        thread_t *current = CURRENT_THREAD;

        ASSERT(current != target);

        interrupt_save_flag(&current->cpu_state.interrupt_flag);
        interrupt_disable_preemption();

        /*
         * save FSBase and KernelGSBase
         * (which holds userland GSBase value at this point)
         */
        current->cpu_state.fsbase = rdmsr(0xc0000100);
        current->cpu_state.gsbase = rdmsr(0xc0000102);

        __thread_do_switch(
            &current->cpu_state.rsp, &current->cpu_state.rip,
            target->cpu_state.rsp, target->cpu_state.rip, current);
}

void
__thread_load_context(thread_t *target)
{
        /* assume interrupt disabled */
        thread_t *prev = CURRENT_THREAD;
        CURRENT_THREAD = target;

        ASSERT(prev != target);

        if (prev && prev->state == THREAD_STATE_EXITED) {
                prev->running = B_FALSE;
        }

        percpu()->tss->rsp0            = (u64)target->kernel_stack;
        percpu()->current_kernel_stack = target->kernel_stack;

        vm_address_space_load(CURRENT_ADDRESS_SPACE);

        /*
         * load userland FSBase and GSBase value
         * (latter goes into KernelGSBase MSR)
         */
        wrmsr(0xc0000100, target->cpu_state.fsbase);
        wrmsr(0xc0000102, target->cpu_state.gsbase);

        interrupt_load_flag(target->cpu_state.interrupt_flag);

        sched_enable();
}

void
__thread_on_terminate(void)
{
        uint rc = atomic_dec_fetch_uint(
            CURRENT_PROCESS->running_thread_count, __ATOMIC_ACQ_REL);
        if (!rc) { __process_on_terminate(); }

        sched_leave(CURRENT_THREAD);
        sched_resched();
        PANIC("__thread_on_terminate reached end");
}

void
__thread_on_sysret(void)
{
        if (atomic_load_boolean(
                CURRENT_THREAD->terminate_flag, __ATOMIC_ACQUIRE)) {
                CURRENT_THREAD->state = THREAD_STATE_EXITED;
                __thread_on_terminate();
        }
}

void
__thread_on_user_iret(void)
{
        if (atomic_load_boolean(
                CURRENT_THREAD->terminate_flag, __ATOMIC_ACQUIRE)) {
                CURRENT_THREAD->state = THREAD_STATE_EXITED;
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

void
thread_terminate(thread_t *thread)
{
        atomic_store_boolean(thread->terminate_flag, B_TRUE, __ATOMIC_RELEASE);
}

#define PID_MAX            32768
#define PID_BITMAP_ENTRIES 32768
#define PID_BITMAP_SIZE    (PID_BITMAP_ENTRIES / 64)

static u64     pid_bitmap[PID_BITMAP_SIZE];
static mutex_t pid_bitmap_lock;

static void
pid_bitmap_set(size_t pos)
{
        ASSERT(pos < PID_BITMAP_ENTRIES);

        size_t idx = pos / 8;
        size_t off = pos % 8;

        ASSERT((pid_bitmap[idx] & (1ull << off)) == 0);

        pid_bitmap[idx] |= (1ull << off);
}

static void
pid_bitmap_clear(size_t pos)
{
        ASSERT(pos < PID_BITMAP_ENTRIES);

        size_t idx = pos / 8;
        size_t off = pos % 8;

        ASSERT((pid_bitmap[idx] & (1ull << off)) != 0);

        pid_bitmap[idx] &= ~(1ull << off);
}

static pid_t
alloc_pid(void)
{
        /* TODO: lockfree */
        mutex_acquire(&pid_bitmap_lock);

        size_t idx;
        size_t off;
        for (idx = 0; idx < PID_BITMAP_SIZE; ++idx) {
                if (pid_bitmap[idx] == ~(0ull)) { continue; }

                for (off = 0; off < 64; ++off) {
                        if (pid_bitmap[idx] & (1ull << off)) { continue; }

                        pid_t ret = idx * 8 + off;
                        pid_bitmap_set(ret);
                        mutex_release(&pid_bitmap_lock);
                        return ret + 1;
                }

                ASSERT(off < 64);
        }

        mutex_release(&pid_bitmap_lock);
        return -1;
}

static void
free_pid(pid_t pid)
{
        mutex_acquire(&pid_bitmap_lock);
        pid_bitmap_clear(pid);
        mutex_release(&pid_bitmap_lock);
}

void
process_init(void)
{
        kmemset(pid_bitmap, 0, sizeof(pid_bitmap));
        mutex_init(&pid_bitmap_lock);

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

        __kernel_proc = process_create(NULL);
        VERIFY(__kernel_proc, "failed to allocate kernel process");

        __kernel_proc->address_space = vm_address_space_create();
        VERIFY(
            __kernel_proc->address_space,
            "failed to allocate address space for kernel proc");
}

process_t *
process_create(process_t *parent)
{
        pid_t pid = alloc_pid();
        if (!pid) return NULL;

        process_t *ret = kmem_alloc(sizeof(process_t));
        if (!ret) {
                free_pid(pid);
                return NULL;
        }

        int kobject_error = KERN_OK;
        if (!kobject_init(ret, &kobject_error)) {
                kmem_free(ret);
                free_pid(pid);
                return NULL;
        }

        mutex_init(&ret->lock);

        ret->pid      = pid;
        ret->next_tid = 0;

        ret->thread_count         = 0;
        ret->running_thread_count = 0;

        ret->wait_count = 0;

        ret->retval = 0;

        ret->reincarnation_flag = B_FALSE;

        list_head_init(&ret->thread_list);
        list_head_init(&ret->child_list_head);

        if (parent) {
                ret->parent = parent;
                list_insert(&ret->sibling_list_node, &parent->child_list_head);
        }

        return ret;
}

void
process_destroy(process_t *process)
{
        ASSERT(list_is_empty(&process->thread_list));
        ASSERT(list_is_empty(&process->child_list_head));

        ASSERT(process->parent);
        mutex_acquire(&process->parent->lock);
        list_remove(&process->sibling_list_node);
        mutex_release(&process->parent->lock);

        if (process->address_space
            && !atomic_dec_fetch_uint(
                process->address_space->ref_count, __ATOMIC_ACQ_REL)) {
                vm_address_space_destroy(process->address_space);
        }

        free_pid(process->pid);
        kmem_free(process);
}

/* Assume holding process lock */
void
process_terminate(process_t *process)
{
        kprintf("kernel process termination: %d\n", process->pid);
        LIST_FOREACH(process->thread_list, p)
        {
                thread_t *t = CONTAINER_OF(p, thread_t, proc_list_node);
                thread_terminate(t);
        }
}

static void
__process_on_terminate(void)
{
        if (!CURRENT_PROCESS->parent) {
                PANIC("process with NULL parent terminated");
        }

        mutex_acquire(&CURRENT_PROCESS->lock);
        kobject_cleanup(CURRENT_PROCESS);

        mutex_acquire(&CURRENT_PROCESS->parent->lock);

        LIST_FOREACH_MUT(CURRENT_PROCESS->child_list_head, p, __next)
        {
                process_t *child =
                    CONTAINER_OF(p, process_t, sibling_list_node);
                mutex_try_acquire(&child->lock);
                child->parent = CURRENT_PROCESS->parent;
                list_remove(&child->sibling_list_node);
                list_insert(
                    &child->sibling_list_node,
                    &CURRENT_PROCESS->parent->child_list_head);
                mutex_release(&child->lock);
        }

        mutex_release(&CURRENT_PROCESS->parent->lock);

        mutex_release(&CURRENT_PROCESS->lock);

}
