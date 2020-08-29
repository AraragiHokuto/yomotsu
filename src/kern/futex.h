#ifndef ORIHIME_FUTEX_H__
#define ORIHIME_FUTEX_H__

#define FUTEX_WAKE_ALL ((size_t)-1)

#ifdef _KERNEL

#include <kern/memory.h>
#include <kern/types.h>

typedef u64 futex_val_t;

void futex_init(void);
void futex_wait(address_space_t *as, void *addr, futex_val_t val);
void futex_wake(address_space_t *as, void *addr, size_t count);

/* alternative interface foir kernel space futexes */
void futex_kwait(futex_val_t *addr, futex_val_t val);
void futex_kwake(futex_val_t *addr, size_t count);

#else /* _KERNEL */

#include <stdint.h>

typedef uint64_t futex_val_t;

#endif /* _KERNEL */

#endif /* ORIHIME_FUTEX_H__ */
