#ifndef RENZAN_SPINLOCK_H__
#define RENZAN_SPINLOCK_H__

#include <k_atomic.h>
#include <k_int.h>

struct spinlock_s {
        atomic_boolean locked;
        uint           owner;
};

typedef struct spinlock_s spinlock_t;

void    spinlock_init(spinlock_t *lock);
boolean spinlock_try_lock(spinlock_t *lock, irqflag_t *flag);
void    spinlock_lock(spinlock_t *lock, irqflag_t *flag);
void    spinlock_unlock(spinlock_t *lock, irqflag_t flag);

#endif /* RENZAN_SPINLOCK_H__ */
