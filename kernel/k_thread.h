/* k_proc.h -- Process implementation */

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

#ifndef __RENZAN_K_THREAD_H__
#define __RENZAN_K_THREAD_H__

#include <k_atomic.h>
#include <k_cap.h>
#include <k_futex.h>
#include <k_int.h>
#include <k_list.h>
#include <k_memory.h>
#include <k_mutex.h>
#include <k_sched.h>

#include <osrt/thread.h>
#include <osrt/types.h>

struct kobject_s;
typedef struct kobject_s kobject_t;

enum THREAD_STATE {
        THREAD_STATE_READY = 1,
        THREAD_STATE_SUSPENDED,
        THREAD_STATE_EXITED
};

enum THREAD_EXCEPTION {
        THREAD_EXCEPTION = 0,
        THREAD_EXCEPTION_DIV_BY_ZERO,
        THREAD_EXCEPTION_PROTECTION,
        THREAD_EXCEPTION_ACCESS_VIOLATION,
};

struct thread_s {
        mutex_t lock;

        tid_t tid;

        u64 retval;

        cap_t root_cnode;

        address_space_t *address_space;

        u64            state;
        atomic_boolean terminate_flag;

        kobject_t *kobjects;
        size_t     kobject_size;

        list_node_t sched_list_node;

        struct thread_s *parent;
        list_node_t      sibling_list_node;
        list_node_t      child_list_head;

        futex_val_t exited_child_count;

        struct {
                void *    rsp;
                void *    rip;
                irqflag_t interrupt_flag;
                void *    simd_state;
                u64       fsbase;
        } cpu_state;

        sched_process_data_t sched_data;

        void *kernel_stack;
};

typedef struct thread_s thread_t;

void      thread_init(void);
thread_t *thread_create(thread_t *t, thread_t *parnet);
void      thread_destroy(thread_t *t);
void      thread_start(thread_t *t, void *entry, void *data);
void      thread_switch_context(thread_t *t);
void      thread_terminate(thread_t *t);
void      thread_raise_exception(thread_t *t, int exception);

#define CURRENT_THREAD        (percpu()->current_thread)
#define CURRENT_ADDRESS_SPACE (CURRENT_THREAD->address_space)

#endif /* __RENZAN_K_PROC_H__ */
