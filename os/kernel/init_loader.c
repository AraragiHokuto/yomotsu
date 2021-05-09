#include <os/kernel/console.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/memory.h>
#include <os/kernel/process.h>
#include <os/kernel/string.h>
#include <os/kernel/types.h>

#include <hal_percpu.h>

#define ELF_MAGIC 0x464c457f

struct elf_ident_s {
        u32 magic;
        u8 class;
        u8 data;
        u8 version;
        u8 osabi;
        u8 abiversion;
        u8 pad[7];
} PACKED;

typedef struct elf_ident_s elf_ident_t;

struct elf_header_s {
        elf_ident_t ident;
        u16         type;
        u16         machine;
        u32         version;
        u64         entry;
        u64         phoff;
        u64         shoff;
        u32         flags;
        u16         ehsize;
        u16         phentsize;
        u16         phnum;
        u16         shentsize;
        u16         shnum;
        u16         shstrndx;
} PACKED;

typedef struct elf_header_s elf_header_t;

struct elf_program_header_s {
        u32 type;
        u32 flags;
        u64 offset;
        u64 vaddr;
        u64 paddr;
        u64 filesz;
        u64 memsz;
        u64 align;
};

typedef struct elf_program_header_s elf_program_header_t;

static uintptr init_begin, init_end;

void
init_set_module(uintptr begin, uintptr end)
{
        init_begin = begin;
        init_end   = end;
}

static void *
init_load_elf(address_space_t *as, byte *p)
{
        elf_header_t *h = (void *)p;

        if (h->ident.magic != ELF_MAGIC) {
                kprintf("invalid magic: 0x%x\n", h->ident.magic);
                return NULL;
        }

        if (h->ident.class != 2) {
                kprintf("wrong class\n");
                return NULL;
        }

        void *entry = (void *)h->entry;

        elf_program_header_t *ph = (void *)(p + h->phoff);

        for (size_t i = 0; i < h->phnum; ++i) {
                if (ph[i].type != 1) /* PT_LOAD */
                        continue;

                kprintf("loading segment to 0x%x\n", ph[i].vaddr);

                byte * vma   = (byte *)ph[i].vaddr;
                size_t memsz = ph[i].memsz;
                while (memsz) {
                        if (!vm_is_mapped(as, vma)) {
                                uintptr pa = pma_alloc(PMA_ZONE_ANY);
                                if (!pa) return NULL;

                                if (!vm_map_page(
                                        as, vma, pa,
                                        VM_PAGE_WRITABLE | VM_PAGE_EXECUTABLE))
                                        return NULL;
                        }

                        if (memsz <= KMEM_PAGE_SIZE)
                                memsz = 0;
                        else
                                memsz -= KMEM_PAGE_SIZE;
                        vma += KMEM_PAGE_SIZE;
                }
                vma = (byte *)ph[i].vaddr;

                kmemset(vma, 0, ph[i].memsz);
                kmemcpy(vma, p + ph[i].offset, ph[i].filesz);
        }

        return entry;
}

void *
init_load(address_space_t *as)
{
        if (!init_begin) return NULL;

        byte *p   = vm_get_vma_for_pma(init_begin);
        void *ret = init_load_elf(as, p);

        return ret;
}

extern void __init_start(void *entry);

void
init_start(process_t *self)
{
        void *entry = init_load(self->address_space);
        VERIFY(entry, "failed to load init at 0x%x", init_begin);

        kprintf("init_start(): starting process\n");

        process_start(self, __init_start, entry);
        self->state = PROCESS_STATE_READY;
        sched_enter(self);
        sched_start();
}
