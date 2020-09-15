#ifndef IZANAMI_SCHED_H__
#define IZANAMI_SCHED_H__

#include <kern/boolean.h>
#include <kern/list.h>

#ifdef _KERNEL

typedef struct thread_s thread_t;

struct sched_percpu_data_s {
        thread_t *starve_scan_thread;
        boolean   sched_enabled;
};

typedef struct sched_percpu_data_s sched_percpu_data_t;

enum SCHED_THREAD_CLASS {
        SCHED_CLASS_IDLE,
        SCHED_CLASS_NORMAL,
        SCHED_CLASS_DRIVER,
        SCHED_CLASS_RT,
        SCHED_CLASS_RT_DRIVER,
};

struct sched_thread_data_s {
        int class;
        uint cpu;
        uint priority;
        u64  quantum;
        u64  last_exec;
};

typedef struct sched_thread_data_s sched_thread_data_t;

void sched_init(void);
void sched_start(void);
void sched_enter(thread_t *thread);
void sched_set_ready(thread_t *thread);
void sched_set_blocking(thread_t *thread);
void sched_leave(thread_t *thread);
void sched_resched();

void sched_disable(void);
void sched_enable(void);

#endif /* _KERNEL */

#endif /* IZANAMI_SCHED_H__ */
