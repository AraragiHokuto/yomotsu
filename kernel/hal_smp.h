#ifndef __RENZAN_HAL_SMP_H__
#define __RENZAN_HAL_SMP_H__

#include <osrt/types.h>

struct cpu_s {
        u32 kern_id;
        u32 lapic;
};

typedef struct cpu_s cpu_t;

uint   smp_current_cpu_id();
cpu_t *smp_get_cpu(size_t id);
uint   smp_cpu_count(void);

#endif /* __RENZAN_HAL_SMP_H__ */
