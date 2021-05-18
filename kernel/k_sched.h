/* k_sched.h -- Scheduler implementation */

/*
 * Copyright 2021 Mosakuji Hokuto <shikieiki@yamaxanadu.org>.
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

#ifndef __RENZAN_K_SCHED_H__
#define __RENZAN_K_SCHED_H__

#include <k_list.h>

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

#endif /* __RENZAN_K_SCHED_H__ */
