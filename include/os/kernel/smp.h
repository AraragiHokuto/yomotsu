#ifndef KAGUYA_SMP_H_
#define KAGUYA_SMP_H_

#include <os/kernel/percpu.h>
#include <os/kernel/types.h>

struct cpu_s {
        u32 kern_id;
        u32 lapic;
};

typedef struct cpu_s cpu_t;

void   smp_init(void);
uint   smp_current_cpu_id();
void   smp_start_all_aps(void);
cpu_t *smp_get_cpu(size_t id);
uint   smp_cpu_count(void);

#endif /* KAGUYA_SMP_H_ */
