#ifndef RENZAN_PERCPU_H_
#define RENZAN_PERCPU_H_

#include <os/kernel/boolean.h>
#include <os/kernel/descriptor.h>
#include <os/kernel/interrupt.h>
#include <os/kernel/memory.h>
#include <os/kernel/process.h>
#include <os/kernel/sched.h>
#include <os/kernel/timer.h>
#include <os/kernel/types.h>

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

#endif /* RENZAN_PERCPU_H_ */
