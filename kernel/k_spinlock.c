#include <hal_smp.h>
#include <k_cdefs.h>
#include <k_spinlock.h>
#include <k_string.h>

void
spinlock_init(spinlock_t *lock)
{
        kmemset(lock, 0, sizeof(lock));
}

boolean
spinlock_try_lock(spinlock_t *lock, irqflag_t *flag)
{
        interrupt_save_flag(flag);
        interrupt_disable_preemption();
        boolean expected = B_FALSE;
        boolean ret      = atomic_cmpxchg_strong_boolean(
            lock->locked, expected, B_TRUE, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);

        if (!ret) {
                interrupt_load_flag(*flag);
        } else {
                lock->owner = smp_current_cpu_id();
        }

        return ret;
}

void
spinlock_lock(spinlock_t *lock, irqflag_t *flag)
{
        interrupt_save_flag(flag);
        interrupt_disable_preemption();

        while (B_TRUE) {
                boolean expected = B_FALSE;
                if (atomic_cmpxchg_weak_boolean(
                        lock->locked, expected, B_TRUE, __ATOMIC_SEQ_CST,
                        __ATOMIC_ACQUIRE)) {
                        break;
                }
        }

        lock->owner = smp_current_cpu_id();
}

void
spinlock_unlock(spinlock_t *lock, irqflag_t flag)
{
        VERIFY(lock->locked, "unlocking an unlocked lock");
        VERIFY(
            lock->owner == smp_current_cpu_id(),
            "%d unlocking spinlock owned by %d", lock->owner,
            smp_current_cpu_id());

        atomic_store_boolean(lock->locked, B_FALSE, __ATOMIC_SEQ_CST);
        interrupt_load_flag(flag);
}
