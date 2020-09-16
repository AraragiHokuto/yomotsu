#include <kern/console.h>
#include <kern/list.h>
#include <kern/macrodef.h>
#include <kern/percpu.h>
#include <kern/process.h>
#include <kern/sched.h>
#include <kern/smp.h>
#include <kern/spinlock.h>
#include <kern/timer.h>

#define TOTAL_LEVELS        64
#define NORMAL_LEVEL_MIN    1
#define NORMAL_LEVEL_MAX    15
#define DRIVER_LEVEL_MIN    16
#define DRIVER_LEVEL_MAX    31
#define RT_LEVEL_MIN        32
#define RT_LEVEL_MAX        47
#define RT_DRIVER_LEVEL_MIN 48
#define RT_DRIVER_LEVEL_MAX 63

struct sched_queue_s {
        spinlock_t  lock;
        atomic_uint process_count;
        list_node_t ready_list[TOTAL_LEVELS];
};

typedef struct sched_queue_s sched_queue_t;
static sched_queue_t *       queue;

extern process_t *__kernel_proc;

static uint
sched_pick_cpu(process_t *t)
{
        /* TODO: weight if we should migrate */
        DONTCARE(t);
        uint ret       = 0;
        uint ret_count = (uint)-1;
        for (uint i = 0; i < smp_cpu_count(); ++i) {
                uint i_count =
                    atomic_load_uint(queue[i].process_count, __ATOMIC_ACQUIRE);
                if (i_count < ret_count) {
                        ret       = i;
                        ret_count = i_count;
                }
        }

        return ret;
}

void
sched_init(void)
{
        queue = kmem_alloc(sizeof(sched_queue_t) * smp_cpu_count());
        VERIFY(queue, "failed to allocate schedule queue");

        kprintf("initializing scheduler\n");
        for (uint i = 0; i < smp_cpu_count(); ++i) {
                spinlock_init(&queue[i].lock);
                for (uint j = 0; j < TOTAL_LEVELS; ++j) {
                        list_head_init(&queue[i].ready_list[j]);
                }
                atomic_store_uint(queue[i].process_count, 0, __ATOMIC_RELEASE);
        }
}

static void
__do_sched_set_ready(process_t *proc)
{
        uint      cpu = sched_pick_cpu(proc);
        irqflag_t flag;
        spinlock_lock(&queue[cpu].lock, &flag);
        list_insert(
            &proc->sched_list_node,
            queue[cpu].ready_list[proc->sched_data.priority].prev);
        queue[cpu].process_count++;
        proc->sched_data.cpu = cpu;
        spinlock_unlock(&queue[cpu].lock, flag);
}

static void
__do_sched_remove_ready(process_t *process)
{
        uint      cpu = process->sched_data.cpu;
        irqflag_t flag;
        spinlock_lock(&queue[cpu].lock, &flag);
        list_remove(&process->sched_list_node);
        queue[cpu].process_count--;
        spinlock_unlock(&queue[cpu].lock, flag);
}

#define UNIT_QUANTUM_NORMAL    20
#define UNIT_QUANTUM_DRIVER    20
#define UNIT_QUANTUM_RT        20
#define UNIT_QUANTUM_RT_DRIVER 20

#define LINEAR_QUANTUM(p, max_l, unit_quantum) \
        (((max_l) - (p) + 1) * (unit_quantum))

#define CALC_QUANTUM(t, c) \
        LINEAR_QUANTUM(t->sched_data.priority, c##_LEVEL_MAX, UNIT_QUANTUM_##c)

static u64
get_quantum(process_t *proc)
{
        switch (proc->sched_data.class) {
        case SCHED_CLASS_IDLE:
                return (u64)-1;
        case SCHED_CLASS_NORMAL:
                return CALC_QUANTUM(proc, NORMAL);
        case SCHED_CLASS_DRIVER:
                return CALC_QUANTUM(proc, DRIVER);
        case SCHED_CLASS_RT:
                return CALC_QUANTUM(proc, RT);
        case SCHED_CLASS_RT_DRIVER:
                return CALC_QUANTUM(proc, RT_DRIVER);
        default:
                PANIC("invalid schedule class");
        }
}

static u64 get_priority(uint class)
{
        switch (class) {
        case SCHED_CLASS_IDLE:
                return 0;
        case SCHED_CLASS_NORMAL:
                return NORMAL_LEVEL_MAX;
        case SCHED_CLASS_DRIVER:
                return DRIVER_LEVEL_MAX;
        case SCHED_CLASS_RT:
                return RT_LEVEL_MAX;
        case SCHED_CLASS_RT_DRIVER:
                return RT_DRIVER_LEVEL_MAX;
        default:
                PANIC("invalid schedule class");
        }
}

static void
clampped_priority_change(process_t *proc, sint delta)
{
        uint p = proc->sched_data.priority + delta;
        switch (proc->sched_data.class) {
        case SCHED_CLASS_IDLE:
                p = 0;
#define CLAMP_CASE(c)                           \
        case SCHED_CLASS_##c:                   \
                if (p > c##_LEVEL_MAX) {        \
                        p = c##_LEVEL_MAX;      \
                } else if (p < c##_LEVEL_MIN) { \
                        p = c##_LEVEL_MIN;      \
                }                               \
                break;

                CLAMP_CASE(NORMAL);
                CLAMP_CASE(DRIVER);
                CLAMP_CASE(RT);
                CLAMP_CASE(RT_DRIVER);
#undef CLAMP_CASE
        default:
                PANIC("invalid schedule class");
        }

        proc->sched_data.priority = p;
}

static void
thread_bump_blocking(process_t *proc)
{
        clampped_priority_change(proc, 1);
}

/* Assume holding queue lock */
static void
__do_sched_update_position(process_t *proc)
{
        list_remove(&proc->sched_list_node);
        list_insert(
            &proc->sched_list_node, queue[smp_current_cpu_id()]
                                        .ready_list[proc->sched_data.priority]
                                        .prev);
}

void
sched_enter(process_t *proc)
{
        proc->sched_data.priority = get_priority(proc->sched_data.class);
        __do_sched_set_ready(proc);
}

void
sched_set_ready(process_t *proc)
{
        __do_sched_set_ready(proc);
}

void
sched_set_blocking(process_t *proc)
{
        __do_sched_remove_ready(proc);
        thread_bump_blocking(proc);
}

void
sched_leave(process_t *proc)
{
        __do_sched_remove_ready(proc);
}

static void
sched_idle_main(void)
{
        interrupt_enable_preemption();
        while (1) asm volatile("hlt");
}

void __process_load_context(process_t *proc);

#define CURRENT_QUEUE (queue + smp_current_cpu_id())
#define READY_LIST(l) (CURRENT_QUEUE->ready_list + (l))

static process_t *
sched_pick_next(void)
{
        process_t *ret = NULL;
        for (int i = TOTAL_LEVELS - 1; i >= 0; --i) {
                if (list_is_empty(READY_LIST(i))) { continue; }

                ret = CONTAINER_OF(
                    READY_LIST(i)->next, process_t, sched_list_node);
                break;
        }

        ASSERT(ret);
        return ret;
}

/* Assume holding queue lock */
static boolean
sched_should_preempt(void)
{
        uint i;
        for (i = TOTAL_LEVELS - 1; i > 0; --i) {
                if (!list_is_empty(READY_LIST(i))) { break; }
        }

        return i > CURRENT_PROCESS->sched_data.priority;
}

void
sched_tick(void *_data)
{
        DONTCARE(_data);

        timer_set_timeout(10, sched_tick, NULL);

        irqflag_t flag;
        spinlock_lock(&CURRENT_QUEUE->lock, &flag);

        if (sched_should_preempt()) {
                process_t *next = sched_pick_next();
                ASSERT(next != CURRENT_PROCESS);
                __do_sched_update_position(CURRENT_PROCESS);
                spinlock_unlock(&CURRENT_QUEUE->lock, flag);

                sched_disable();
                next->sched_data.quantum = get_quantum(next);
                process_switch_context(next);
                return;
        }

        CURRENT_PROCESS->sched_data.quantum -= 10;
        if (!CURRENT_PROCESS->sched_data.quantum) {
                clampped_priority_change(CURRENT_PROCESS, -1);
                __do_sched_update_position(CURRENT_PROCESS);
                process_t *next = sched_pick_next();
                if (next != CURRENT_PROCESS) {
                        spinlock_unlock(&CURRENT_QUEUE->lock, flag);

                        sched_disable();
                        next->sched_data.quantum = get_quantum(next);
                        process_switch_context(next);
                        return;
                }
        }

        spinlock_unlock(&CURRENT_QUEUE->lock, flag);
}

void
sched_resched(void)
{
        if (UNLIKELY(!percpu()->sched_data.sched_enabled)) { return; }

        irqflag_t flag;
        spinlock_lock(&CURRENT_QUEUE->lock, &flag);
        process_t *next = sched_pick_next();
        spinlock_unlock(&CURRENT_QUEUE->lock, flag);

        ASSERT(next->state == PROCESS_STATE_READY);

        next->sched_data.quantum = get_quantum(next);
        if (next != CURRENT_PROCESS) {
                sched_disable();
                process_switch_context(next);
        }
}

void
sched_start(void)
{
        process_t *idle_proc           = process_create(NULL);
        idle_proc->address_space       = __kernel_proc->address_space;
        idle_proc->sched_data.class    = SCHED_CLASS_IDLE;
        idle_proc->sched_data.priority = 0;
        idle_proc->sched_data.quantum  = get_quantum(idle_proc);
        idle_proc->state               = PROCESS_STATE_READY;

        irqflag_t flag;
        spinlock_lock(&queue[smp_current_cpu_id()].lock, &flag);
        list_insert(
            &idle_proc->sched_list_node,
            &queue[smp_current_cpu_id()].ready_list[0]);
        __process_load_context(idle_proc);
        spinlock_unlock(&queue[smp_current_cpu_id()].lock, flag);

        kprintf(
            "sched_start(): starting sched on CPU%d\n", smp_current_cpu_id());

        timer_set_timeout(10, sched_tick, NULL);

        sched_idle_main();
}

void
sched_disable(void)
{
        percpu()->sched_data.sched_enabled = B_FALSE;
}

void
sched_enable(void)
{
        percpu()->sched_data.sched_enabled = B_TRUE;
}
