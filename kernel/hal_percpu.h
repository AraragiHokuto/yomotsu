/* hal_percpu.h -- HAL Per-CPU data implementation */

/*
 * Copyright 2021 Mosakuji Hokuto.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __RENZAN_HAL_PERCPU_H__
#define __RENZAN_HAL_PERCPU_H__

#include <k_descriptor.h>
#include <k_int.h>
#include <k_memory.h>
#include <k_proc.h>
#include <k_sched.h>
#include <k_timer.h>

/* Per-CPU data */
typedef struct percpu_data_s {
        void *__self; /* self pointer */
        u64   cpuid;  /* current processor id */

        gdt_t *gdt;
        tss_t *tss;

        process_t *current_process;
        void *     current_kernel_stack;

        /* timer heap */
        timer_heap_element_t *timer_heap;
        timer_heap_element_t *timer_heap_end;
        size_t                timer_heap_size;

        boolean timer_defer_pending;

        sched_percpu_data_t sched_data;

        interrupt_percpu_data_t interrupt_data;
} percpu_data_t;

percpu_data_t *percpu(void);

#endif /* __RENZAN_HAL_PERCPU_H__ */
