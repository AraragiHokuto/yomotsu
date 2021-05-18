#ifndef __RENZAN_K_PROC_H__
#define __RENZAN_K_PROC_H__

#include <osrt/types.h>
#include <osrt/process.h>

#include <k_atomic.h>
#include <k_futex.h>
#include <k_int.h>
#include <k_list.h>
#include <k_memory.h>
#include <k_mutex.h>
#include <k_sched.h>

struct kobject_s;
typedef struct kobject_s kobject_t;

enum PROCESS_STATE {
        PROCESS_STATE_READY = 1,
        PROCESS_STATE_BLOCKED,
        PROCESS_STATE_EXITED
};

enum PROCESS_EXCEPTION {
	PROC_EXCEPTION_NONE = 0,
	PROC_EXCEPTION_DIV_BY_ZERO,
	PROC_EXCEPTION_PROTECTION,
	PROC_EXCEPTION_ACCESS_VIOLATION,
};

struct process_s {
        mutex_t lock;

        pid_t pid;

        u64 retval;

        address_space_t *address_space;

        u64            state;
        atomic_boolean terminate_flag;

        kobject_t *kobjects;
        size_t     kobject_size;

        list_node_t sched_list_node;

        struct process_s *parent;
        list_node_t       sibling_list_node;
        list_node_t       child_list_head;

        futex_val_t       exited_child_count;

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

typedef struct process_s process_t;

void process_init(void);

process_t *process_create(process_t *parnet);
void       process_destroy(process_t *process);
void       process_start(process_t *process, void *entry, void *data);
void       process_switch_context(process_t *process);
void       process_terminate(process_t *process);
void       process_raise_exception(process_t *process, int exception);

#define CURRENT_PROCESS       (percpu()->current_process)
#define CURRENT_ADDRESS_SPACE (CURRENT_PROCESS->address_space)

#endif /* __RENZAN_K_PROC_H__ */
