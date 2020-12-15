#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <os/runtime/memory.h>
#include <os/runtime/misc.h>
#include <os/runtime/process.h>
#include <os/runtime/syscall.h>

/* ELF loader */
typedef uint64_t elf64_addr;
typedef uint64_t elf64_off;
typedef uint16_t elf64_half;
typedef uint32_t elf64_word;
typedef int32_t  elf64_sword;
typedef uint64_t elf64_xword;
typedef uint64_t elf64_sxword;

enum {
        EI_MAG0       = 0,
        EI_MAG1       = 1,
        EI_MAG2       = 2,
        EI_MAG3       = 3,
        EI_CLASS      = 4,
        EI_DATA       = 5,
        EI_VERSION    = 6,
        EI_OSABI      = 7,
        EI_ABIVERSION = 8,
        EI_PAD        = 9,
        EI_NIDENT     = 16
};

enum {
        ELFCLASS32 = 1,
        ELFCLASS64 = 2,
};

enum {
        ELFDATA2LSB = 1,
        ELFDATA2MSB = 2,
};

enum { ELFOSABI_SYSV = 0, ELFOSABI_HPUX = 1, ELFOSABI_STANDALONE = 255 };

enum {
        ET_NONE   = 0,
        ET_REL    = 1,
        ET_EXEC   = 2,
        ET_DYN    = 3,
        ET_CORE   = 4,
        ET_LOOS   = 0xfe00,
        ET_HIOS   = 0xfeff,
        ET_LOPROC = 0xff00,
        ET_HIPROC = 0xffff
};

typedef struct elf_ehdr_s {
        unsigned char e_ident[16];
        elf64_half    e_type;
        elf64_half    e_machine;
        elf64_word    e_version;
        elf64_addr    e_entry;
        elf64_off     e_phoff;
        elf64_off     e_shoff;
        elf64_word    e_flags;
        elf64_half    e_ehsize;
        elf64_half    e_phentsize;
        elf64_half    e_phnum;
        elf64_half    e_shentsize;
        elf64_half    e_shnum;
        elf64_half    e_shstmdx;
} elf_ehdr_t;

enum {
        PT_NULL    = 0,
        PT_LOAD    = 1,
        PT_DYNAMIC = 2,
        PT_INTERP  = 3,
        PT_NOTE    = 4,
        PT_SHLIB   = 5,
        PT_PHDR    = 6,
        PT_LOOS    = 0x60000000,
        PT_HIOS    = 0x6fffffff,
        PT_LOPROC  = 0x70000000,
        PT_HIPROX  = 0x7fffffff
};

#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4
#define PF_MASKOS   0x00ff0000
#define PF_MASKPROC 0xff000000

typedef struct elf_phdr_s {
        elf64_word  p_type;
        elf64_word  p_flags;
        elf64_off   p_offset;
        elf64_addr  p_vaddr;
        elf64_addr  p_paddr;
        elf64_xword p_filesz;
        elf64_xword p_memsz;
        elf64_xword p_align;
} elf_phdr_t;

static bool
verify_ehdr(elf_ehdr_t *ehdr)
{
        if (*(uint32_t *)&ehdr->e_ident[EI_MAG0] != 0x464c457f) {
                return false;
        }

        return true;
}

#define PS_ALIGNUP(x) (((((x)-1) / __PAGE_SIZE) + 1) * __PAGE_SIZE)

static bool
load_segment(kobject_t as, uint8_t *elf, elf_phdr_t *phdr)
{
        assert(phdr->p_type == PT_LOAD);

        /* allocate page for segment */
        int flags = 0;
        if (phdr->p_flags & PF_W) { flags |= PAGE_ATTR_WRITABLE; }
        if (phdr->p_flags & PF_X) { flags |= PAGE_ATTR_EXECUTABLE; }

        size_t mem_size = PS_ALIGNUP(phdr->p_memsz);

        intptr_t this_vaddr =
            syscall_mmap(AS_CURRENT, 0, mem_size, PAGE_ATTR_WRITABLE);
        if (this_vaddr < 0) {
                errno = -this_vaddr;
                return false;
        }

        memcpy((void *)this_vaddr, elf + phdr->p_offset, phdr->p_filesz);

        int64_t ret = syscall_mtransfer(
            as, AS_CURRENT, phdr->p_vaddr, this_vaddr, phdr->p_memsz, flags);
        if (ret < 0) {
                syscall_munmap(AS_CURRENT, this_vaddr, phdr->p_memsz);
                errno = -ret;
                return false;
        }

        return true;
}

/*
 * Load an executable elf file from memory into address space
 * Return entry address
 */
uintptr_t
elf_load_from_memory(kobject_t as, void *elf)
{
        if (!verify_ehdr(elf)) {
                errno = EINVAL;
                return 0;
        }

        elf_ehdr_t *ehdr = elf;
        elf_phdr_t *phdr = (void *)((uint8_t *)elf + ehdr->e_phoff);

        for (size_t i = 0; i < ehdr->e_phnum; ++i) {
                if (phdr[i].p_type != PT_LOAD) continue;
                bool ret = load_segment(as, elf, &phdr[i]);
                if (!ret) return 0;
        }

        return ehdr->e_entry;
}

_Noreturn void
process_exit(int64_t retval)
{
        syscall_process_exit(retval);
}

#define STACK_SIZE (8 * 1024 * 1024) /* 8MB */

static uintptr_t
_setup_as_for_proc(kobject_t as, void *elf, int argc, char **argv)
{
        /* load elf into address space */
        uintptr_t entry = elf_load_from_memory(as, elf);
        if (!entry) { return 0; }

        /* setup init structure */
        size_t init_data_size = sizeof(__libos_process_init_data_t);

        size_t arg_total_len = 0;
        for (int i = 0; i < argc; ++i) { arg_total_len += strlen(argv[i]) + 1; }
        init_data_size += arg_total_len;

        size_t argv_len = (argc + 1) * sizeof(char *);
        init_data_size += argv_len;

        init_data_size = ALIGNUP(init_data_size, __PAGE_SIZE);

        uintptr_t dst_init_data_addr = __USERLAND_END_ADDR - init_data_size;

        intptr_t init_data =
            syscall_mmap(AS_CURRENT, 0, init_data_size, __MMAP_WRITABLE);
        if (init_data < 0) {
                errno = -init_data;
                return 0;
        }

        __libos_process_init_data_t *data =
            (void *)(init_data + init_data_size
		     - sizeof(__libos_process_init_data_t));

        char *p = (char *)data - arg_total_len;

        char **argv_p = (void *)((char *)data - arg_total_len - argv_len);
        for (int i = 0; i < argc; ++i) {
                argv_p[i] =
                    (void *)((uintptr_t)p - init_data + dst_init_data_addr);
                strcpy(p, argv[i]);
                p += strlen(argv[i]) + 1;
        }
        argv_p[argc] = NULL;

        data->argc = argc;
        data->argv =
            (void *)((uintptr_t)argv_p - init_data + dst_init_data_addr);
        data->rsp = ALIGNDOWN(dst_init_data_addr - 1, 16);

        int64_t ret = syscall_mtransfer(
            as, AS_CURRENT, dst_init_data_addr, init_data, init_data_size, 0);
        if (ret < 0) {
                errno = -ret;
                return 0;
        }

        /* map pages for stack */
        /* TODO guard page */
        uintptr_t stack_bottom_addr =
            __USERLAND_END_ADDR - init_data_size - STACK_SIZE;

        ret = syscall_mmap(as, stack_bottom_addr, STACK_SIZE, __MMAP_WRITABLE);
        if (ret < 0) {
                errno = -ret;
                return 0;
        }

        assert(ret == stack_bottom_addr);

        return entry;
}

bool
process_wait(pid_t pid, int64_t *ret)
{
        process_state_t state;
        int64_t         syscall_ret = syscall_process_wait(pid, &state);
        if (syscall_ret < 0) {
                errno = -syscall_ret;
                return false;
        }

        *ret = state.retval;
        return true;
}

pid_t
process_spawn_from_memory(void *elf, int argc, char **argv)
{
        kobject_t as = syscall_as_create();
        if (as < 0) {
                errno = -as;
                return -1;
        }

        uintptr_t entry = _setup_as_for_proc(as, elf, argc, argv);
        if (!entry) {
                syscall_as_destroy(as);
                return -1;
        }

        pid_t ret = syscall_process_spawn(as, (void *)entry);
        if (ret < 0) {
                syscall_as_destroy(as);
                errno = -ret;
                return -1;
        }

        return ret;
}
