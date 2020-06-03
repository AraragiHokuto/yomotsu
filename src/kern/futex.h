#ifndef ORIHIME_FUTEX_H__
#define ORIHIME_FUTEX_H__

#include <kern/types.h>
#include <kern/memory.h>

typedef u64	futex_val_t;

#define	FUTEX_WAKE_ALL	((size_t)-1)

#ifdef _KERNEL

void	futex_init(void);
void	futex_wait(address_space_t *as, void *addr, futex_val_t val);
void	futex_wake(address_space_t *as, void *addr, size_t count);

/* alternative interface foir kernel space futexes */
void	futex_kwait(futex_val_t *addr, futex_val_t val);
void	futex_kwake(futex_val_t *addr, size_t count);

#endif	/* _KERNEL */

#endif	/* ORIHIME_FUTEX_H__ */
