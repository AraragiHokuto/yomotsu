#ifndef ORIHIME_PROCESS_H__
#define ORIHIME_PROCESS_H__

#include <kern/types.h>

#ifdef _KERNEL

#include <kern/atomic.h>
#include <kern/interrupt.h>
#include <kern/list.h>
#include <kern/memory.h>
#include <kern/mutex.h>
#include <kern/sched.h>
#include <kern/futex.h>

typedef s64 pid_t;
typedef s64 tid_t;

typedef u64 __u64;

struct kboject_s;
typedef struct kobject_s kobject_t;

struct process_s {
        mutex_t lock;

        pid_t pid;
        tid_t next_tid;

        atomic_uint thread_count;
        atomic_uint running_thread_count;

        atomic_uint wait_count;

        list_node_t thread_list;

        u64              retval;
        address_space_t *address_space;

        atomic_boolean reincarnation_flag;

        kobject_t *kobjects;
        size_t     kobject_size;

        struct process_s *parent;
        list_node_t       sibling_list_node;
        list_node_t       child_list_head;
};

typedef struct process_s process_t;

enum THREAD_STATE {
        THREAD_STATE_READY = 1,
        THREAD_STATE_BLOCKED,
        THREAD_STATE_EXITED
};

struct thread_s {
        mutex_t lock;

        tid_t tid;

        u64 state;

        list_node_t sched_list_node;
        list_node_t proc_list_node;

        process_t *container;

        atomic_boolean running;
        atomic_boolean terminate_flag;

        u64 retval;

        atomic_uint wait_count;

        struct {
                void *    rsp;
                void *    rip;
                irqflag_t interrupt_flag;
                void *    simd_state;
                u64       fsbase;
                u64       gsbase;
        } cpu_state;

        sched_thread_data_t sched_data;

        void *kernel_stack;
};

typedef struct thread_s thread_t;

void process_init(void);

thread_t *thread_create(process_t *container);
void      thread_destroy(thread_t *thread);
void      thread_start(thread_t *thread, void *entry, void *data);
void      thread_switch_context(thread_t *thread);
void      thread_terminate(thread_t *thread);

process_t *process_create(process_t *parnet);
void       process_destroy(process_t *process);
void       process_terminate(process_t *process);

#define CURRENT_THREAD        (percpu()->current_thread)
#define CURRENT_PROCESS       (CURRENT_THREAD->container)
#define CURRENT_ADDRESS_SPACE (CURRENT_PROCESS->address_space)

#else

#include <stdint.h>

typedef int64_t pid_t;
typedef int64_t tid_t;

#endif /* _KERNEL */

struct process_state_s {
        pid_t process_id;
        __u64 retval;
};

typedef struct process_state_s process_state_t;

struct thread_state_s {
        tid_t thread_id;
        __u64 retval;
};

typedef struct thread_state_s thread_state_t;

#endif /* ORIHIME_PROCESS_H__ */
