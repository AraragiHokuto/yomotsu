#include <os/kernel/acpi.h>
#include <os/kernel/asm.h>
#include <os/kernel/boolean.h>
#include <os/kernel/console.h>
#include <os/kernel/interrupt.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/memory.h>
#include <os/kernel/string.h>
#include <os/kernel/types.h>

#include <hal_percpu.h>

typedef struct idt_gate_s {
        u16 off_low;
        u16 selector;
        u8  ist : 3, resv1 : 5;
        u8  type : 4, z : 1, dpl : 2, p : 1;
        u16 off_mid;
        u32 off_high;
        u32 ign;
} PACKED idt_gate_t;

#define IDT_GATES 256
static idt_gate_t idt[IDT_GATES];

#define IRQ_OFFSET 32

#define IRQ_STACK_SIZE 16384 /* 16KB */

static void
lidt(u16 limit, void *base)
{
        typedef struct idtr_s {
                u16 limit;
                u64 base;
        } PACKED idtr_t;

        idtr_t idtr = {.limit = limit, .base = (u64)base};

        asm volatile("lidt %0" ::"m"(idtr));
}

void
isr_load(void)
{
        /* load IDT */
        lidt(sizeof(idt), idt);
}

void
isr_init(void)
{
        /* fill IDT */
#define DECL_ISR(_no)                                    \
        do {                                             \
                extern void _isr##_no();                 \
                uintptr     isroff = (uintptr)_isr##_no; \
                idt[_no]           = (idt_gate_t){       \
                    .off_low  = isroff,        \
                    .selector = 0x08,          \
                    .ist      = 0,             \
                    .resv1    = 0,             \
                    .type     = 0x0E,          \
                    .z        = 0,             \
                    .dpl      = 0,             \
                    .p        = 1,             \
                    .off_mid  = isroff >> 16,  \
                    .off_high = isroff >> 32,  \
                    .ign      = 0,             \
                };                             \
        } while (0)

        DECL_ISR(0);
        DECL_ISR(1);
        DECL_ISR(2);
        DECL_ISR(3);
        DECL_ISR(4);
        DECL_ISR(5);
        DECL_ISR(6);
        DECL_ISR(7);
        DECL_ISR(8);
        DECL_ISR(9);
        DECL_ISR(10);
        DECL_ISR(11);
        DECL_ISR(12);
        DECL_ISR(13);
        DECL_ISR(14);
        DECL_ISR(15);
        DECL_ISR(16);
        DECL_ISR(17);
        DECL_ISR(18);
        DECL_ISR(19);
        DECL_ISR(20);
        DECL_ISR(21);
        DECL_ISR(22);
        DECL_ISR(23);
        DECL_ISR(24);
        DECL_ISR(25);
        DECL_ISR(26);
        DECL_ISR(27);
        DECL_ISR(28);
        DECL_ISR(29);
        DECL_ISR(30);
        DECL_ISR(31);

#undef DECL_ISR

#define DECL_IRQ(_no)                                    \
        do {                                             \
                extern void _irq##_no();                 \
                uintptr     irqoff = (uintptr)_irq##_no; \
                idt[_no]           = (idt_gate_t){       \
                    .off_low  = irqoff,        \
                    .selector = 0x08,          \
                    .ist      = 0,             \
                    .resv1    = 0,             \
                    .type     = 0x0E,          \
                    .z        = 0,             \
                    .dpl      = 0,             \
                    .p        = 1,             \
                    .off_mid  = irqoff >> 16,  \
                    .off_high = irqoff >> 32,  \
                    .ign      = 0,             \
                };                             \
        } while (0)

        DECL_IRQ(32);
        DECL_IRQ(33);
        DECL_IRQ(34);
        DECL_IRQ(35);
        DECL_IRQ(36);
        DECL_IRQ(37);
        DECL_IRQ(38);
        DECL_IRQ(39);
        DECL_IRQ(40);
        DECL_IRQ(41);
        DECL_IRQ(42);
        DECL_IRQ(43);
        DECL_IRQ(44);
        DECL_IRQ(45);
        DECL_IRQ(46);
        DECL_IRQ(47);
        DECL_IRQ(48);
        DECL_IRQ(49);
        DECL_IRQ(50);
        DECL_IRQ(51);
        DECL_IRQ(52);
        DECL_IRQ(53);
        DECL_IRQ(54);
        DECL_IRQ(55);
        DECL_IRQ(56);
        DECL_IRQ(57);
        DECL_IRQ(58);
        DECL_IRQ(59);
        DECL_IRQ(60);
        DECL_IRQ(61);
        DECL_IRQ(62);
        DECL_IRQ(63);
        DECL_IRQ(64);
        DECL_IRQ(65);
        DECL_IRQ(66);
        DECL_IRQ(67);
        DECL_IRQ(68);
        DECL_IRQ(69);
        DECL_IRQ(70);
        DECL_IRQ(71);
        DECL_IRQ(72);
        DECL_IRQ(73);
        DECL_IRQ(74);
        DECL_IRQ(75);
        DECL_IRQ(76);
        DECL_IRQ(77);
        DECL_IRQ(78);
        DECL_IRQ(79);
        DECL_IRQ(80);
        DECL_IRQ(81);
        DECL_IRQ(82);
        DECL_IRQ(83);
        DECL_IRQ(84);
        DECL_IRQ(85);
        DECL_IRQ(86);
        DECL_IRQ(87);
        DECL_IRQ(88);
        DECL_IRQ(89);
        DECL_IRQ(90);
        DECL_IRQ(91);
        DECL_IRQ(92);
        DECL_IRQ(93);
        DECL_IRQ(94);
        DECL_IRQ(95);
        DECL_IRQ(96);
        DECL_IRQ(97);
        DECL_IRQ(98);
        DECL_IRQ(99);
        DECL_IRQ(100);
        DECL_IRQ(101);
        DECL_IRQ(102);
        DECL_IRQ(103);
        DECL_IRQ(104);
        DECL_IRQ(105);
        DECL_IRQ(106);
        DECL_IRQ(107);
        DECL_IRQ(108);
        DECL_IRQ(109);
        DECL_IRQ(110);
        DECL_IRQ(111);
        DECL_IRQ(112);
        DECL_IRQ(113);
        DECL_IRQ(114);
        DECL_IRQ(115);
        DECL_IRQ(116);
        DECL_IRQ(117);
        DECL_IRQ(118);
        DECL_IRQ(119);
        DECL_IRQ(120);
        DECL_IRQ(121);
        DECL_IRQ(122);
        DECL_IRQ(123);
        DECL_IRQ(124);
        DECL_IRQ(125);
        DECL_IRQ(126);
        DECL_IRQ(127);
        DECL_IRQ(128);
        DECL_IRQ(129);
        DECL_IRQ(130);
        DECL_IRQ(131);
        DECL_IRQ(132);
        DECL_IRQ(133);
        DECL_IRQ(134);
        DECL_IRQ(135);
        DECL_IRQ(136);
        DECL_IRQ(137);
        DECL_IRQ(138);
        DECL_IRQ(139);
        DECL_IRQ(140);
        DECL_IRQ(141);
        DECL_IRQ(142);
        DECL_IRQ(143);
        DECL_IRQ(144);
        DECL_IRQ(145);
        DECL_IRQ(146);
        DECL_IRQ(147);
        DECL_IRQ(148);
        DECL_IRQ(149);
        DECL_IRQ(150);
        DECL_IRQ(151);
        DECL_IRQ(152);
        DECL_IRQ(153);
        DECL_IRQ(154);
        DECL_IRQ(155);
        DECL_IRQ(156);
        DECL_IRQ(157);
        DECL_IRQ(158);
        DECL_IRQ(159);
        DECL_IRQ(160);
        DECL_IRQ(161);
        DECL_IRQ(162);
        DECL_IRQ(163);
        DECL_IRQ(164);
        DECL_IRQ(165);
        DECL_IRQ(166);
        DECL_IRQ(167);
        DECL_IRQ(168);
        DECL_IRQ(169);
        DECL_IRQ(170);
        DECL_IRQ(171);
        DECL_IRQ(172);
        DECL_IRQ(173);
        DECL_IRQ(174);
        DECL_IRQ(175);
        DECL_IRQ(176);
        DECL_IRQ(177);
        DECL_IRQ(178);
        DECL_IRQ(179);
        DECL_IRQ(180);
        DECL_IRQ(181);
        DECL_IRQ(182);
        DECL_IRQ(183);
        DECL_IRQ(184);
        DECL_IRQ(185);
        DECL_IRQ(186);
        DECL_IRQ(187);
        DECL_IRQ(188);
        DECL_IRQ(189);
        DECL_IRQ(190);
        DECL_IRQ(191);
        DECL_IRQ(192);
        DECL_IRQ(193);
        DECL_IRQ(194);
        DECL_IRQ(195);
        DECL_IRQ(196);
        DECL_IRQ(197);
        DECL_IRQ(198);
        DECL_IRQ(199);
        DECL_IRQ(200);
        DECL_IRQ(201);
        DECL_IRQ(202);
        DECL_IRQ(203);
        DECL_IRQ(204);
        DECL_IRQ(205);
        DECL_IRQ(206);
        DECL_IRQ(207);
        DECL_IRQ(208);
        DECL_IRQ(209);
        DECL_IRQ(210);
        DECL_IRQ(211);
        DECL_IRQ(212);
        DECL_IRQ(213);
        DECL_IRQ(214);
        DECL_IRQ(215);
        DECL_IRQ(216);
        DECL_IRQ(217);
        DECL_IRQ(218);
        DECL_IRQ(219);
        DECL_IRQ(220);
        DECL_IRQ(221);
        DECL_IRQ(222);
        DECL_IRQ(223);
        DECL_IRQ(224);
        DECL_IRQ(225);
        DECL_IRQ(226);
        DECL_IRQ(227);
        DECL_IRQ(228);
        DECL_IRQ(229);
        DECL_IRQ(230);
        DECL_IRQ(231);
        DECL_IRQ(232);
        DECL_IRQ(233);
        DECL_IRQ(234);
        DECL_IRQ(235);
        DECL_IRQ(236);
        DECL_IRQ(237);
        DECL_IRQ(238);
        DECL_IRQ(239);
        DECL_IRQ(240);
        DECL_IRQ(241);
        DECL_IRQ(242);
        DECL_IRQ(243);
        DECL_IRQ(244);
        DECL_IRQ(245);
        DECL_IRQ(246);
        DECL_IRQ(247);
        DECL_IRQ(248);
        DECL_IRQ(249);
        DECL_IRQ(250);
        DECL_IRQ(251);
        DECL_IRQ(252);
        DECL_IRQ(253);
        DECL_IRQ(254);
        DECL_IRQ(255);
#undef DECL_IRQ

        /* disable 8259 PIC -- we will be using IOAPIC */
        outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 0x28);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0xA1, 0xff);
        outb(0x21, 0xff);
}

static char *isr_shortname[32] = {
    "#DE", "#DB", "NMI", "#BP", "OF", "#BR", "#UD", "#NM", "#DF", "", "#TS",
    "#NP", "#SS", "#GP", "#PF", "",   "#MF", "#AC", "#MC", "#XF", "", "",
    "",    "",    "",    "",    "",   "",    "",    "#VC", "#SX", ""};

static void
page_fault_handler()
{
        uintptr pf_addr;
        asm volatile("movq %%cr4, %0" : "=r"(pf_addr));
        PANIC("Page Fault at 0x%x", pf_addr);
}

void
isr_handler(uint vector, uint errcode, uintptr return_rip)
{
        kprintf(
            "ISR handler called: vector = %d, errcode = %d,"
            " return_rip = 0x%x\n",
            vector, errcode, return_rip);

        switch (vector) {
        case 14:
                page_fault_handler();
                break;
        default:
                PANIC(
                    "ISR handler not implemented: %d %s", vector,
                    isr_shortname[vector]);
                break;
        }
}

static boolean
check_apic(void)
{
        u64 rdx;
        asm volatile("cpuid" : "=d"(rdx) : "a"(1) : "rcx", "rbx");

        return (rdx & (1 << 9)) != 0;
}

static byte *apic_reg_base = NULL;

u32
apic_read_reg(uintptr offset)
{
        return *(u32 volatile *)(apic_reg_base + offset);
}

void
apic_write_reg(uintptr offset, u32 value)
{
        *(u32 volatile *)(apic_reg_base + offset) = value;
}

extern void timer_init(void);

static uintptr
find_lapic_base(void)
{
        uintptr base = acpi_madt_lapic_info()->lapic_addr;
        kprintf("\t32bit APIC base addr at 0x%x\n", base);

        for (acpi_madt_record_t *r = acpi_madt_first_record(); r;
             r                     = acpi_madt_next_record(r)) {
                if (r->header.type != 5) continue;
                kprintf(
                    "\t64bit APIC base addr override: 0x%x\n",
                    r->lapic_addr_override.lapic_pma);
                base = r->lapic_addr_override.lapic_pma;
        }

        return base;
}

#define PERCPU (percpu()->interrupt_data)

struct irq_defer_queue_s {
        irq_defer_callback_t callback;
        void *               data;
};

#define DEFER_QUEUE_INIT_SIZE 32

static void
defer_queue_init(void)
{
        PERCPU.defer_queue =
            kmem_alloc(DEFER_QUEUE_INIT_SIZE * sizeof(irq_defer_queue_t));
        VERIFY(PERCPU.defer_queue, "OOM in defer_queue_init(): defer queue");
        PERCPU.defer_queue_end  = PERCPU.defer_queue;
        PERCPU.defer_queue_size = DEFER_QUEUE_INIT_SIZE;
}

static void
defer_queue_expand(void)
{
        size_t             new_size  = PERCPU.defer_queue_size * 3 / 2;
        irq_defer_queue_t *new_queue = kmem_realloc(
            PERCPU.defer_queue, new_size * sizeof(irq_defer_queue_t));

        /* XXX: how to handle OOM here? */
        VERIFY(new_queue, "OOM in defer_queue_expand()");

        size_t occupied = PERCPU.defer_queue_end - PERCPU.defer_queue;

        PERCPU.defer_queue      = new_queue;
        PERCPU.defer_queue_end  = new_queue + occupied;
        PERCPU.defer_queue_size = new_size;
}

static void
defer_queue_push(irq_defer_queue_t *element)
{
        if (PERCPU.defer_queue_end - PERCPU.defer_queue
            >= PERCPU.defer_queue_size) {
                defer_queue_expand();
        }

        irq_defer_queue_t *ptr = PERCPU.defer_queue_end++;
        kmemcpy(ptr, element, sizeof(*element));
}

static void
defer_queue_pop(irq_defer_queue_t *out)
{
        kmemcpy(out, --PERCPU.defer_queue_end, sizeof(*out));
}

static boolean
defer_queue_empty(void)
{
        return PERCPU.defer_queue_end == PERCPU.defer_queue;
}

void
interrupt_init_bsp(void)
{
        PERCPU.irq_handlers =
            kmem_alloc(sizeof(irq_handler_t) * (IDT_GATES - IRQ_OFFSET));
        VERIFY(
            PERCPU.irq_handlers, "OOM in interrupt_init_bsp(): irq_handlers");

        if (!check_apic()) {
                kprintf("APIC unavailable\n");
                return;
        }

        defer_queue_init();

        uintptr apic_base = find_lapic_base();
        ASSERT(apic_base);

        apic_reg_base = vm_get_vma_for_pma(apic_base);
        if (!vm_is_mapped(NULL, apic_reg_base)) {
                vm_map_page(
                    NULL, apic_reg_base, apic_base,
                    VM_PAGE_WRITABLE | VM_PAGE_PRIVILEGED | VM_PAGE_GLOBAL);
        }

        if (!apic_reg_base) {
                kprintf("mapping LAPIC base failed\n");
                return;
        }

        u32 val = apic_read_reg(APIC_REG_SPURIOUS);
        apic_write_reg(APIC_REG_SPURIOUS, val | 0x1FF);
        apic_write_reg(APIC_REG_DEST_FORMAT, 0xFFFFFFFF);
        apic_write_reg(APIC_REG_TASK_PRIORITY, 0x00000000);

        interrupt_enable_preemption();

        timer_init_bsp();
}

void
interrupt_init_ap(void)
{
        PERCPU.irq_handlers =
            kmem_alloc(sizeof(irq_handler_t) * (IDT_GATES - IRQ_OFFSET));
        VERIFY(PERCPU.irq_handlers, "OOM in interrupt_init_ap(): irq_handlers");

        defer_queue_init();

        ASSERT(apic_reg_base);

        u32 val = apic_read_reg(APIC_REG_SPURIOUS);
        apic_write_reg(APIC_REG_SPURIOUS, val | 0x1FF);
        apic_write_reg(APIC_REG_DEST_FORMAT, 0xFFFFFFFF);
        apic_write_reg(APIC_REG_TASK_PRIORITY, 0x00000000);

        interrupt_enable_preemption();
        timer_init_ap();
}

void
irq_handler(u64 irq)
{
        irq_handler_t h = PERCPU.irq_handlers[irq - IRQ_OFFSET];
        if (h) {
                h(irq);
        } else {
                kprintf("unhandled irq: %d\n", irq);
        }

        apic_write_reg(APIC_REG_EOI, 0);
}

void
interrupt_set_handler(u64 vector, irq_handler_t handler)
{
        VERIFY(
            vector >= IRQ_OFFSET && vector < IDT_GATES, "vector out of range");
        PERCPU.irq_handlers[vector - IRQ_OFFSET] = handler;
}

void
interrupt_clear_handler(u64 vector)
{
        VERIFY(
            vector >= IRQ_OFFSET && vector < IDT_GATES, "vector out of range");
        PERCPU.irq_handlers[vector - IRQ_OFFSET] = NULL;
}

void
interrupt_enable_preemption(void)
{
        asm volatile("sti");
}

void
interrupt_disable_preemption(void)
{
        asm volatile("cli");
}

#define IF_BIT 9

void
interrupt_save_flag(irqflag_t *flag)
{
        u64 rflags;
        asm volatile(
            "pushfq\n"
            "popq %0\n"
            : "=r"(rflags));

        *flag = !!(rflags & (1 << IF_BIT));
}

void
interrupt_load_flag(irqflag_t flag)
{
        if (flag)
                interrupt_enable_preemption();
        else
                interrupt_disable_preemption();
}
