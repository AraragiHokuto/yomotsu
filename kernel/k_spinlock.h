/* XXX to be reimplmented */

#ifndef RENZAN_SPINLOCK_H__
#define RENZAN_SPINLOCK_H__

#include <k_atomic.h>
#include <k_int.h>

struct spinlock_s {
        atomic_boolean locked;
#ifdef _KDEBUG
        uint owner;
#endif /* _KDEBUG */
};

typedef struct spinlock_s spinlock_t;

void    spinlock_init(spinlock_t *lock);
boolean spinlock_try_lock(spinlock_t *lock);
void    spinlock_lock(spinlock_t *lock);
void    spinlock_unlock(spinlock_t *lock);

#endif /* RENZAN_SPINLOCK_H__ */
