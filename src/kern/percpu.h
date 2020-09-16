#ifndef ORIHIME_PERCPU_H_
#define ORIHIME_PERCPU_H_

#include <kern/boolean.h>
#include <kern/descriptor.h>
#include <kern/interrupt.h>
#include <kern/memory.h>
#include <kern/process.h>
#include <kern/sched.h>
#include <kern/timer.h>
#include <kern/types.h>

/* Per-CPU data */
typedef struct percpu_data_s {
        void *__self; /* self pointer */
        u64   cpuid;  /* current processor id */

        gdt_t *gdt;
        tss_t *tss;

        process_t *current_process;
        void *    current_kernel_stack;

        /* timer heap */
        timer_heap_element_t *timer_heap;
        timer_heap_element_t *timer_heap_end;
        size_t                timer_heap_size;

        boolean timer_defer_pending;

        sched_percpu_data_t sched_data;

        interrupt_percpu_data_t interrupt_data;
} percpu_data_t;

void           percpu_init(void);
percpu_data_t *percpu(void);

#endif /* ORIHIME_PERCPU_H_ */
