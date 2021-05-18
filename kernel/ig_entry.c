/* ig_entry.c -- Ignitor entry point */

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

#include <hal_percpu.h>
#include <hal_smp.h>
#include <ig_smp.h>
#include <k_acpi.h>
#include <k_console.h>
#include <k_futex.h>
#include <k_int.h>
#include <k_memory.h>
#include <k_port.h>
#include <k_port_ifce.h>
#include <k_proc.h>
#include <k_sched.h>
#include <k_simd.h>
#include <k_syscall.h>
#include <k_timer.h>

void init_start(process_t *self);

#define STR(x)  _STR(x)
#define _STR(x) #x

void
ig_entry_bsp(u64 rsdp, u64 xsdp)
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
        ig_smp_init();

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
        ig_smp_start_all_aps();

        address_space_t *as = vm_address_space_create();
        ASSERT(as);

        process_t *proc = process_create(NULL);
        ASSERT(proc);
        proc->address_space    = as;
        proc->sched_data.class = SCHED_CLASS_NORMAL;

        vm_address_space_load(proc->address_space);

        init_start(proc);

        PANIC("ig_entry_bsp reached bottom!");
}

void
ig_entry_ap(void)
{
        descriptor_init();
        isr_load();
        interrupt_init_ap();
        syscall_init();
        process_init();
        sched_start();

        PANIC("ig_entry_bsp reached bottom!");
}
