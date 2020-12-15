#include <os/kernel/console.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/percpu.h>

struct stackframe_s {
        struct stackframe_s *rbp;
        uintptr              rip;
};

typedef struct stackframe_s stackframe_t;

struct sym_table_entry_s {
        u64 address;
        u64 str_table_pos;
};

typedef struct sym_table_entry_s sym_table_entry_t;

struct sym_table_s {
        u64               str_table_offset;
        sym_table_entry_t table[];
};

typedef struct sym_table_s sym_table_t;

static boolean have_sym_table = B_FALSE;

static sym_table_t *sym_table     = NULL;
static const char * sym_str_table = NULL;

static uintptr sym_start, sym_end;

static void
symbol_table_load(void *begin, void *end)
{
        byte *       ptr = begin;
        sym_table_t *t   = begin;

        byte *_end = end;

        if (ptr + t->str_table_offset >= _end) return;

        sym_table     = t;
        sym_str_table = (const char *)ptr + t->str_table_offset;
}

static const char *
sym_table_lookup(u64 address, u64 *offset)
{
        if (!have_sym_table) return NULL;

        symbol_table_load(
            vm_get_vma_for_pma(sym_start), vm_get_vma_for_pma(sym_end));

        uint begin = 0;
        uint end   = (sym_table->str_table_offset - 8) / 16;

        ASSERT(end != 0);

        if (address < sym_table->table[begin].address) { return NULL; }

        while (1) {
                ASSERT(end != begin);
                if (end - begin == 1) {
                        sym_table_entry_t *ptr = &sym_table->table[begin];
                        *offset                = address - ptr->address;
                        return &sym_str_table[ptr->str_table_pos];
                }

                u64                mid = (end - begin) / 2 + begin;
                sym_table_entry_t *ptr = &sym_table->table[mid];
                if (ptr->address > address) {
                        end = mid;
                } else {
                        begin = mid;
                }
        }
}

void
__kern_sym_table_load(uintptr begin, uintptr end)
{
        have_sym_table = B_TRUE;
        sym_start      = begin;
        sym_end        = end;
}

void
__stack_trace(void)
{
        stackframe_t *rbp;
        asm volatile("movq %%rbp, %0" : "=r"(rbp));

        kprintf("\nstacktrace:\n");
        while (rbp) {
                const char *sym;
                u64         offset;

                sym = sym_table_lookup(rbp->rip, &offset);

                if (sym) {
                        kprintf("\tat 0x%x <%s+0x%x>\n", rbp->rip, sym, offset);
                } else {
                        kprintf("\tat 0x%x\n", rbp->rip);
                }
                rbp = rbp->rbp;
        }
}

void NORETURN
__panic(const char *func, uint lineno, const char *fmt, ...)
{
        asm volatile("cli");
        asm volatile("xchgw %bx, %bx"); /* Bochs MAGIC BREAK */

        kprintf("%s:%d:[CPU%d]PANIC:\t", func, lineno, percpu()->cpuid);
        __builtin_va_list ap;
        __builtin_va_start(ap, fmt);

        kvprintf(ap, fmt);

        __stack_trace();

        asm volatile("cli");
        while (1) { asm volatile("hlt"); }
}
