#include <kern/futex.h>
#include <kern/list.h>
#include <kern/smp.h>
#include <kern/spinlock.h>
#include <kern/user_memory.h>

struct futex_bucket_s {
        spinlock_t  lock;
        list_node_t head;
        uint        waiters;
};

typedef struct futex_bucket_s futex_bucket_t;

struct futex_entry_s {
        uintptr     addr;
        process_t *  proc;
        list_node_t node;
};

typedef struct futex_entry_s futex_entry_t;

static futex_bucket_t *futex_hash;
static size_t          futex_size;

static size_t
futex_calc_hash(uintptr addr)
{
        return addr % futex_size;
}

static u64
round_to_power_of_2(u64 val)
{
        return val == 1 ? 1 : (1 << (64 - __builtin_clzll(val) + 1));
}

void
futex_init(void)
{
        futex_size = round_to_power_of_2(256 * smp_cpu_count());

        /*
         * Notice we allocate 2 * futex_size buckets here
         * Those buckets will be divided into two halves,
         * one part for userland, the other for kernel space
         */
        futex_hash = kmem_alloc(sizeof(futex_bucket_t) * futex_size * 2);

        for (size_t i = 0; i < futex_size * 2; ++i) {
                spinlock_init(&futex_hash[i].lock);
                list_head_init(&futex_hash[i].head);
                futex_hash[i].waiters = 0;
        }
}

void
futex_wait(address_space_t *as, void *addr, futex_val_t val)
{
        mutex_acquire(&as->lock);

        if (!user_memory_check_read(as, addr, sizeof(futex_val_t))) {
                /* Access violation */
                mutex_release(&as->lock);
                mutex_acquire(&CURRENT_PROCESS->lock);
                process_terminate(CURRENT_PROCESS);
                mutex_release(&CURRENT_PROCESS->lock);
                sched_resched();
                return;
        }

        uintptr      paddr = vm_get_pma_by_vma(as, addr);
        futex_val_t *p     = vm_get_vma_for_pma(paddr);

        /* enqueue */
        futex_bucket_t *bucket = &futex_hash[futex_calc_hash(paddr)];
        irqflag_t       flag;
        spinlock_lock(&bucket->lock, &flag);
        ++bucket->waiters;

        if (atomic_load_u64(*p, __ATOMIC_SEQ_CST) == val) {
                futex_entry_t entry;
                entry.addr   = paddr;
                entry.proc = CURRENT_PROCESS;
                list_insert(&entry.node, &bucket->head);

                CURRENT_PROCESS->state = PROCESS_STATE_BLOCKED;
                sched_set_blocking(CURRENT_PROCESS);

                mutex_release(&as->lock);
                spinlock_unlock(&bucket->lock, flag);

                sched_resched();

                ASSERT(CURRENT_PROCESS->state != PROCESS_STATE_BLOCKED);
        } else {
                --bucket->waiters;
                spinlock_unlock(&bucket->lock, flag);
                mutex_release(&as->lock);
        }
}

void
futex_kwait(futex_val_t *addr, futex_val_t val)
{
        futex_bucket_t *bucket =
            &futex_hash[futex_calc_hash((uintptr)addr) + futex_size];

        irqflag_t flag;
        spinlock_lock(&bucket->lock, &flag);

        atomic_inc_fetch_uint(bucket->waiters, __ATOMIC_SEQ_CST);
        /* --- barrier --- */

        if (atomic_load_u64(*addr, __ATOMIC_SEQ_CST) == val) {
                futex_entry_t entry;
                entry.addr   = (uintptr)addr;
                entry.proc = CURRENT_PROCESS;
                list_insert(&entry.node, &bucket->head);

                CURRENT_PROCESS->state = PROCESS_STATE_BLOCKED;
                sched_set_blocking(CURRENT_PROCESS);

                spinlock_unlock(&bucket->lock, flag);

                sched_resched();

                ASSERT(CURRENT_PROCESS->state != PROCESS_STATE_BLOCKED);
        } else {
                --bucket->waiters;
                spinlock_unlock(&bucket->lock, flag);
        }
}

void
__do_futex_wake(size_t key, u64 addr, uint count)
{
        ASSERT(count);

        futex_bucket_t *bucket = &futex_hash[key];

        if (!atomic_load_uint(bucket->waiters, __ATOMIC_SEQ_CST)) { return; }
        /* --- barrier --- */

        irqflag_t flag;
        spinlock_lock(&bucket->lock, &flag);

        LIST_FOREACH_MUT(bucket->head, p, __next)
        {
                futex_entry_t *entry = CONTAINER_OF(p, futex_entry_t, node);
                if (entry->addr != addr) { continue; }

                list_remove(&entry->node);
                entry->proc->state = PROCESS_STATE_READY;
                sched_set_ready(entry->proc);

                --count;
                --bucket->waiters;

                if (!bucket->waiters || !count) break;
        }

        spinlock_unlock(&bucket->lock, flag);
}

void
futex_wake(address_space_t *as, void *addr, size_t count)
{
        mutex_acquire(&as->lock);
        if (!user_memory_check_read(as, addr, sizeof(futex_val_t))) {
                mutex_release(&as->lock);
                mutex_acquire(&CURRENT_PROCESS->lock);
                process_terminate(CURRENT_PROCESS);
                mutex_release(&CURRENT_PROCESS->lock);
                return;
        }

        uintptr paddr = vm_get_pma_by_vma(as, addr);
        mutex_release(&as->lock);

        __do_futex_wake(futex_calc_hash(paddr), paddr, count);
}

void
futex_kwake(futex_val_t *addr, size_t count)
{
        __do_futex_wake(
            futex_calc_hash((uintptr)addr) + futex_size, (uintptr)addr, count);
}
