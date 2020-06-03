#ifndef IZANAMI_SPINLOCK_H__
#define IZANAMI_SPINLOCK_H__

#include <kern/interrupt.h>
#include <kern/atomic.h>

struct spinlock_s {
	atomic_boolean	locked;
	uint	owner;
};

typedef struct spinlock_s	spinlock_t;

void	spinlock_init(spinlock_t *lock);
boolean	spinlock_try_lock(spinlock_t *lock, irqflag_t *flag);
void	spinlock_lock(spinlock_t *lock, irqflag_t *flag);
void	spinlock_unlock(spinlock_t *lock, irqflag_t flag);

#endif	/* IZANAMI_SPINLOCK_H__ */
