#include <hal_percpu.h>
#include <k_descriptor.h>
#include <k_memory.h>
#include <k_string.h>

void
descriptor_init(void)
{
        gdt_t *gdt = kmem_alloc(sizeof(gdt_t));
        VERIFY(gdt, "OOM");

        kmemset(gdt, 0, sizeof(gdt_t));

        gdt->null_desc = 0xffff;

        gdt->code_kern.p   = 1;
        gdt->code_kern.l   = 1;
        gdt->code_kern.dpl = 0;
        gdt->code_kern.mb3 = 3;

        gdt->data_kern.p   = 1;
        gdt->data_kern.mb1 = 1;
        /*
         * Bochs require data.w to be set even under long mode.
         * Not sure why.
         */
        gdt->data_kern.w = 1;

        gdt->code_user.p   = 1;
        gdt->code_user.l   = 1;
        gdt->code_user.dpl = 3;
        gdt->code_user.mb3 = 3;

        gdt->data_user.p   = 1;
        gdt->data_user.mb1 = 1;
        gdt->data_user.w   = 1;
        gdt->data_user.dpl = 3;

        gdtr_t gdtr = {.limit = sizeof(gdt_t), .addr = gdt};

        asm volatile("lgdtq %0" ::"m"(gdtr));

        percpu()->gdt = gdt;
}

void
descriptor_load_tss(tss_t *tss)
{
        percpu()->gdt->tss.limit_l = sizeof(tss_t);
        percpu()->gdt->tss.base_l1 = (u64)tss;
        percpu()->gdt->tss.base_l2 = (u64)tss >> 24;
        percpu()->gdt->tss.base_h  = (u64)tss >> 32;
        percpu()->gdt->tss.p       = 1;
        percpu()->gdt->tss.type    = 9;

        asm volatile("ltr %0" ::"a"((u16)0x28));
}
