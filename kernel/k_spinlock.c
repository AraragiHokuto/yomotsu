/* XXX to be reimplemented */

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
spinlock_try_lock(spinlock_t *lock)
{
        boolean expected = B_FALSE;
        boolean ret      = atomic_cmpxchg_strong_boolean(
            lock->locked, expected, B_TRUE, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE);

#ifdef _KDEBUG
        if (ret) { lock->owner = smp_current_cpu_id(); }
#endif /* _KDEBUG */

        return ret;
}

void
spinlock_lock(spinlock_t *lock)
{
        while (B_TRUE) {
                boolean expected = B_FALSE;
                if (atomic_cmpxchg_weak_boolean(
                        lock->locked, expected, B_TRUE, __ATOMIC_SEQ_CST,
                        __ATOMIC_ACQUIRE)) {
                        break;
                }
        }

#ifdef _KDEBUG
        lock->owner = smp_current_cpu_id();
#endif	/* _KDEBUG */
}

void
spinlock_unlock(spinlock_t *lock)
{
#ifdef _KDEBUG
        VERIFY(lock->locked, "unlocking an unlocked lock");
        VERIFY(
            lock->owner == smp_current_cpu_id(),
            "%d unlocking spinlock owned by %d", lock->owner,
            smp_current_cpu_id());
#endif	/* _KDEBUG */

        atomic_store_boolean(lock->locked, B_FALSE, __ATOMIC_SEQ_CST);
}
