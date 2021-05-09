#include <os/kernel/acpi.h>
#include <os/kernel/console.h>
#include <os/kernel/futex.h>
#include <os/kernel/interrupt.h>
#include <os/kernel/memory.h>
#include <os/kernel/percpu.h>
#include <os/kernel/port.h>
#include <os/kernel/port_ifce.h>
#include <os/kernel/process.h>
#include <os/kernel/sched.h>
#include <os/kernel/simd.h>
#include <os/kernel/smp.h>
#include <os/kernel/syscall.h>
#include <os/kernel/timer.h>

void init_start(process_t *self);

#define STR(x) _STR(x)
#define _STR(x) #x

void
kmain_bsp(u64 rsdp, u64 xsdp)
{
        kprintf("Renzan Igniter\n");
	kprintf("rev. " STR(__OSC_BUILD_CI) " build " STR(__OSC_BUILD_TS) "\n");
        kprintf("Initializing MM\n");
        vm_init();
        kprintf("Initializing Kernel Heap\n");
        kmem_heap_init();
        kprintf("Initializing Primitive ISR handlers\n");
        isr_init();
        isr_load();

        if (xsdp) {
                kprintf("loading XSDT\n");
                acpi_load_xsdt(xsdp);
        } else if (rsdp) {
                kprintf("loading RSDT\n");
                acpi_load_rsdt(rsdp);
        }

        kprintf("Initializing Interrupt Handling\n");
        interrupt_init_bsp();

        kprintf("Initializing SMP\n");
        smp_init();

        kprintf("Initializing Process\n");
        descriptor_init();
        futex_init();
        syscall_init();
        process_init();
        sched_init();
        simd_init();

        kprintf("Initalizing Kernel Interfaces\n");
        port_init();
        port_ifce_init();
        syscall_def();

        kprintf("Starting all APs\n");
        smp_start_all_aps();

        address_space_t *as = vm_address_space_create();
        ASSERT(as);

        process_t *proc = process_create(NULL);
        ASSERT(proc);
        proc->address_space = as;
        proc->sched_data.class = SCHED_CLASS_NORMAL;

        vm_address_space_load(proc->address_space);

        init_start(proc);

        PANIC("kmain reached bottom!");
}

void
kmain_ap(void)
{
        descriptor_init();
        isr_load();
        interrupt_init_ap();
        syscall_init();
        process_init();
        sched_start();

        PANIC("kmain reached bottom!");
}
