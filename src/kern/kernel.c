#include <kern/acpi.h>
#include <kern/console.h>
#include <kern/futex.h>
#include <kern/interrupt.h>
#include <kern/memory.h>
#include <kern/percpu.h>
#include <kern/port.h>
#include <kern/port_ifce.h>
#include <kern/process.h>
#include <kern/sched.h>
#include <kern/simd.h>
#include <kern/smp.h>
#include <kern/syscall.h>
#include <kern/timer.h>

void init_start(thread_t *self);

void
kmain_bsp(u64 rsdp, u64 xsdp)
{
        kprintf("Kaguya -- Non-Unix Microkernel\n");
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

        vm_address_space_load(proc->address_space);

        thread_t *thread = thread_create(proc);
        ASSERT(thread);
        thread->sched_data.class = SCHED_CLASS_NORMAL;

        init_start(thread);

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
