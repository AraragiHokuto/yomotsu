#ifndef RENZAN_SCHED_H__
#define RENZAN_SCHED_H__


#include <k_list.h>

#ifdef _KERNEL

typedef struct process_s process_t;

struct sched_percpu_data_s {
        // process_t *starve_scan_process;
        boolean    sched_enabled;
};

typedef struct sched_percpu_data_s sched_percpu_data_t;

enum SCHED_PROC_CLASS {
        SCHED_CLASS_IDLE,
        SCHED_CLASS_NORMAL,
        SCHED_CLASS_DRIVER,
        SCHED_CLASS_RT,
        SCHED_CLASS_RT_DRIVER,
};

struct sched_process_data_s {
        int class;
        uint cpu;
        uint priority;
        u64  quantum;
        u64  last_exec;
};

typedef struct sched_process_data_s sched_process_data_t;

void sched_init(void);
void sched_start(void);
void sched_enter(process_t *proc);
void sched_set_ready(process_t *proc);
void sched_set_blocking(process_t *proc);
void sched_leave(process_t *proc);
void sched_resched();

void sched_disable(void);
void sched_enable(void);

#endif /* _KERNEL */

#endif /* RENZAN_SCHED_H__ */
