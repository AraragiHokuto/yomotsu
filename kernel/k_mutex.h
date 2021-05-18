#ifndef RENZAN_MUTEX_H__
#define RENZAN_MUTEX_H__

#include <k_atomic.h>


#ifdef _KERNEL

struct mutex_s {
        atomic_ptr owner;
};

typedef struct mutex_s mutex_t;

void    mutex_init(mutex_t *mutex);
boolean mutex_try_acquire(mutex_t *mutex);
void    mutex_acquire(mutex_t *mutex);
void    mutex_release(mutex_t *mutex);
void    mutex_acquire_dual(mutex_t *m1, mutex_t *m2);
void    mutex_release_dual(mutex_t *m1, mutex_t *m2);

#endif /* _KERNEL */

#endif /* RENZAN_MUTEX_H__ */
