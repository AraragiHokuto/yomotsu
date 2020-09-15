#include <kern/acpi.h>
#include <kern/asm.h>
#include <kern/boolean.h>
#include <kern/console.h>
#include <kern/interrupt.h>
#include <kern/macrodef.h>
#include <kern/memory.h>
#include <kern/percpu.h>
#include <kern/smp.h>
#include <kern/string.h>
#include <kern/timer.h>

percpu_data_t         bsp_percpu_data asm("bsp_percpu_data");
static percpu_data_t *__ap_percpu_data;

static uint __next_cpuid = 1;

void
percpu_init_ap(void)
{
        kmemset(__ap_percpu_data, 0, sizeof(percpu_data_t));
        __ap_percpu_data->__self = __ap_percpu_data;

        __ap_percpu_data->cpuid = __next_cpuid++;

        wrmsr(0xc0000101, (u64)__ap_percpu_data);
}

percpu_data_t *
percpu(void)
{
        void *ret;
        asm volatile("movq %%gs:0, %0" : "=r"(ret));
        return ret;
}

atomic_boolean __ap_started = B_FALSE;
void *         __ap_stack;

extern void _ap_trampoline;
extern void _ap_start;

static u64 flat_gdt[] = {
    0, 0x00CF9A000000FFFF, /* code */
    0x00CF92000000FFFF,    /* text */
};

static struct {
        u16 limit;
        u64 base;
} PACKED flat_gdt_ptr = {
    .limit = sizeof(flat_gdt), .base = 0x7000 - sizeof(flat_gdt)};

static void
smp_start_ap(u32 target_lapic_id)
{
        atomic_store_boolean(__ap_started, B_FALSE, __ATOMIC_RELEASE);

        kprintf("\tSMP: starting processor %d\n", target_lapic_id);

        /* set target lapic */
        u32 icr1_value = apic_read_reg(APIC_REG_ICR1);
        icr1_value &= ~(0xF << 24);
        icr1_value |= (target_lapic_id & 0xF) << 24;
        apic_write_reg(APIC_REG_ICR1, icr1_value);

        /* send IPI */
	kprintf("\tSMP: send IPI to processor %d\n", target_lapic_id);
        apic_write_reg(APIC_REG_ICR0, 0x00004500);

        /* wait 10ms */
        timer_spin_wait(10);

        /* setup flat GDT for AP */
        ASSERT(sizeof(flat_gdt) == 24);
        ASSERT(sizeof(flat_gdt_ptr) == 10);

        kmemcpy(
            (void *)(0x7000 - sizeof(flat_gdt) - sizeof(flat_gdt_ptr)),
            &flat_gdt_ptr, sizeof(flat_gdt_ptr));
        kmemcpy(
            (void *)(0x7000) - sizeof(flat_gdt), flat_gdt, sizeof(flat_gdt));

        /* setup stack for AP */
        __ap_stack = kmem_alloc(16384);

        /* setup percpu for AP */
        __ap_percpu_data = kmem_alloc(sizeof(percpu_data_t));

        /* copy trampoline code to 0x7000 */
        kmemcpy(
            (void *)0x7000, &_ap_trampoline,
            (uintptr)&_ap_start - (uintptr)&_ap_trampoline);

        /* send SIPI */
	kprintf("\tSMP: send SIPI to processor %d\n", target_lapic_id);
        apic_write_reg(APIC_REG_ICR0, 0x00004607);

        /* wait 1ms */
        timer_spin_wait(1);

        if (!atomic_load_boolean(__ap_started, __ATOMIC_ACQUIRE)) {
                kprintf("\tSMP:AP started flag not set; try again\n");
                apic_write_reg(APIC_REG_ICR0, 0x00004607);

                /* wait 1s */
                timer_spin_wait(1000);

                if (!__ap_started) {
                        kprintf("\tSMP:AP started flag not set; abort\n");
                        kmem_free(__ap_stack);
                        return;
                }
        }

        kprintf("\tSMP: AP started flag set\n");
}

extern void kmain_ap(void);

static uint   __cpu_count = 0;
static cpu_t *__cpus;

void
smp_ap_entry(void)
{
        percpu_init_ap();

        __cpus[percpu()->cpuid].kern_id = percpu()->cpuid;
        __cpus[percpu()->cpuid].lapic   = apic_read_reg(APIC_REG_ID) >> 24;

        atomic_store_boolean(__ap_started, B_TRUE, __ATOMIC_RELEASE);
        kprintf("AP %d started\n", percpu()->cpuid);

        kmain_ap();
}

void
smp_init(void)
{
        /* parse acpi table and find out all cpus */
        for (acpi_madt_record_t *r = acpi_madt_first_record(); r;
             r                     = acpi_madt_next_record(r)) {
                if (r->header.type != 0) continue;
                __cpu_count++;
        }

        kprintf("\tSMP: total cpus found: %d\n", __cpu_count);

        __cpus = kmem_alloc(sizeof(cpu_t) * __cpu_count);
        VERIFY(__cpus, "failed to alloc cpu table");
}

void
smp_start_all_aps(void)
{
        u32 self_lapic_id = apic_read_reg(APIC_REG_ID) >> 24;
        kprintf("\tSMP: self LAPIC id: %d\n", self_lapic_id);

        for (acpi_madt_record_t *r = acpi_madt_first_record(); r;
             r                     = acpi_madt_next_record(r)) {
                if (r->header.type != 0) continue;

                kprintf(
                    "\tSMP: processor found: Processor ID = %d,"
                    " LAPIC ID = %d\n",
                    r->lapic.processor_id, r->lapic.apic_id);

                if (r->lapic.apic_id == self_lapic_id) continue;

                smp_start_ap(r->lapic.apic_id);
        }
}

uint
smp_current_cpu_id(void)
{
        return percpu()->cpuid;
}

uint
smp_cpu_count(void)
{
        return __cpu_count;
}
