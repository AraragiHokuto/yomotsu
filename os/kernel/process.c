#include <os/kernel/asm.h>
#include <os/kernel/console.h>
#include <os/kernel/error.h>
#include <os/kernel/kobject.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/memory.h>
#include <os/kernel/mutex.h>
#include <os/kernel/percpu.h>
#include <os/kernel/process.h>
#include <os/kernel/simd.h>
#include <os/kernel/string.h>

#define KERNEL_STACK_SIZE 16384 /* 16KB */

extern void __process_do_switch(
    void *dst_rsp, void *dst_rip, void *src_rsp, void *src_rip,
    process_t *current);
extern void __process_do_start(
    void *rsp, void *rip, void *entry, void *data, process_t *current);

process_t *__kernel_proc;

static void __process_on_terminate(void);

void
__process_save_context(process_t *target)
{
        process_t *current = CURRENT_PROCESS;

        ASSERT(current != target);

        interrupt_save_flag(&current->cpu_state.interrupt_flag);
        interrupt_disable_preemption();

        /* save FSBase */
        current->cpu_state.fsbase = rdmsr(0xc0000100);

        __process_do_switch(
            &current->cpu_state.rsp, &current->cpu_state.rip,
            target->cpu_state.rsp, target->cpu_state.rip, current);
}

void
__process_load_context(process_t *target)
{
        /* assume interrupt disabled */
        process_t *prev = CURRENT_PROCESS;
        CURRENT_PROCESS = target;

        ASSERT(prev != target);

        if (prev && prev->terminate_flag) {
                prev->state = PROCESS_STATE_EXITED;

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
__process_on_sysret(void)
{
        if (atomic_load_boolean(
                CURRENT_PROCESS->terminate_flag, __ATOMIC_ACQUIRE)) {
                __process_on_terminate();
        }
}

void
__process_on_user_iret(void)
{
        if (atomic_load_boolean(
                CURRENT_PROCESS->terminate_flag, __ATOMIC_ACQUIRE)) {
                __process_on_terminate();
        }
}

void
process_switch_context(process_t *target)
{
        __process_save_context(target);
}

void
process_start(process_t *target, void *entry, void *data)
{
        target->cpu_state.rsp = target->kernel_stack;
        __process_do_start(
            &target->cpu_state.rsp, &target->cpu_state.rip, entry, data,
            target);
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

/* assume holding parent's lock */
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

        byte *kernel_stack = kmem_alloc(KERNEL_STACK_SIZE);
        if (!kernel_stack) {
                kmem_free(ret);
                free_pid(pid);
                return NULL;
        }

        byte *simd_state = simd_alloc_state_area();
        if (!simd_state) {
                kmem_free(kernel_stack);
                kmem_free(ret);
                return NULL;
        }

        int kobject_error = KERN_OK;
        if (!kobject_init(ret, &kobject_error)) {
                simd_free_state_area(simd_state);
                kmem_free(kernel_stack);
                kmem_free(ret);
                free_pid(pid);
                return NULL;
        }

        byte *kernel_rsp   = kernel_stack + KERNEL_STACK_SIZE - 8;
        *(u64 *)kernel_rsp = 0;

        mutex_init(&ret->lock);

        ret->pid          = pid;
        ret->retval       = 0;

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
process_destroy(process_t *process)
{
        ASSERT(list_is_empty(&process->child_list_head));

        ASSERT(process->parent);
        list_remove(&process->sibling_list_node);

        if (process->address_space
            && !atomic_dec_fetch_uint(
                process->address_space->ref_count, __ATOMIC_ACQ_REL)) {
                vm_address_space_destroy(process->address_space);
        }

        kmem_free((byte*)process->kernel_stack - KERNEL_STACK_SIZE + 8);
        simd_free_state_area(process->cpu_state.simd_state);

        free_pid(process->pid);
        kmem_free(process);
}

/* Assume holding process lock */
void
process_terminate(process_t *process)
{
        atomic_store_boolean(process->terminate_flag, B_TRUE, __ATOMIC_RELEASE);
}

void
process_raise_exception(process_t *process, int exception)
{
	/* stub */
	kprintf("Process killed due to exception: %d\n", exception);
	process_terminate(process);
}

static void
__process_on_terminate(void)
{
        if (!CURRENT_PROCESS->parent) {
                PANIC("process with NULL parent terminated");
        }

	sched_disable();

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

        ++CURRENT_PROCESS->parent->exited_child_count;
        futex_kwake(&CURRENT_PROCESS->parent->exited_child_count, 1);

        mutex_release(&CURRENT_PROCESS->parent->lock);

        mutex_release(&CURRENT_PROCESS->lock);

        sched_leave(CURRENT_PROCESS);
	sched_enable();
        sched_resched();

        PANIC("PROCESS_ON_TERMINATE reached end");
}
