#include <os/kernel/acpi.h>
#include <os/kernel/console.h>
#include <os/kernel/cdefs.h>
#include <os/kernel/memory.h>
#include <os/kernel/string.h>

con_driver_t *vga_text_init();

typedef struct PACKED mb2_info_header_s {
        u32 total_size;
        u32 reserved;
} PACKED mb2_info_header_t;

typedef struct mb2_info_tag_header_s {
        u32 type;
        u32 size;
} PACKED mb2_info_tag_header_t;

typedef struct mb2_info_module_s {
        u32  type;
        u32  size;
        u32  start;
        u32  end;
        char string[];
} PACKED mb2_info_module_t;

typedef struct mb2_info_mem_map_entry_s {
        u64 base_addr;
        u64 length;
        u32 type;
        u32 reserved;
} PACKED mb2_info_mem_map_entry_t;

typedef struct mb2_info_mem_map_s {
        u32 type;
        u32 size;
        u32 entry_size;
        u32 entry_version;
} PACKED mb2_info_mem_map_t;

typedef struct mb2_info_acpi_rsdp_s {
        u32  type;
        u32  size;
        char sign[8];
        u8   cksum;
        char oemid[6];
        u8   rev;
        u32  rsdt_addr;
} PACKED mb2_info_acpi_rsdp_t;

typedef struct mb2_info_acpi_rsdp2_s {
        mb2_info_acpi_rsdp_t legacy;
        u32                  length;
        u64                  xsdt_addr;
        u8                   cksum;
        u8                   reserved[3];
} PACKED mb2_info_acpi_rsdp2_t;

enum MB2_INFO_TYPES {
        MB2_INFO_CMDLINE                 = 1,
        MB2_INFO_LOADER_NAME             = 2,
        MB2_INFO_MODULES                 = 3,
        MB2_INFO_BASIC_MEMORY            = 4,
        MB2_INFO_BOOT_DEVICE             = 5,
        MB2_INFO_MEMORY_MAP              = 6,
        MB2_INFO_VBE                     = 7,
        MB2_INFO_FB                      = 8,
        MB2_INFO_ELF_SYMBOLS             = 9,
        MB2_INFO_APM_TABLE               = 10,
        MB2_INFO_EFI32_ST_PTR            = 11,
        MB2_INFO_EFI64_ST_PTR            = 12,
        MB2_INFO_SMBIOS_PTR              = 13,
        MB2_INFO_ACPI_RSDP1              = 14,
        MB2_INFO_ACPI_RSDP2              = 15,
        MB2_INFO_NETWORK                 = 16,
        MB2_INFO_EFI_MEMMAP              = 17,
        MB2_INFO_EFI_BS_NOT_TERMINATED   = 18,
        MB2_INFO_EFI32_IMAGE_HANDLER_PTR = 19,
        MB2_INFO_EFI64_IMAGE_HANDLER_PTR = 20,
        MB2_INFO_IMG_LOAD_BASE_ADDR      = 21
};

#define PREMAPPED_END (32 * 1024 * 1024)

void __kern_sym_table_load(uintptr begin, uintptr end);
void init_set_module(uintptr begin, uintptr end);

extern void _pma_mark_usable(uintptr base, size_t size);
extern void _pma_mark_reserved(uintptr base, size_t size);

static void
ig_mb2_load_module(byte *p)
{
        kprintf("loading module\n");
        mb2_info_module_t *mp = (void *)p;

        ASSERT(mp->type == 3);
        kprintf("\tmodule: start = 0x%x, end = 0x%x\n", mp->start, mp->end);
        kprintf("\tmodule string = %s\n", mp->string);

        if (kstrequ(6, "srcmap", mp->string)) {
                __kern_sym_table_load(mp->start, mp->end);
        } else if (kstrequ(4, "init", mp->string)) {
                init_set_module(mp->start, mp->end);
        }

        _pma_mark_reserved(mp->start, mp->end - mp->start);
}

static void
ig_mb2_load_mem_map(byte *p)
{
        kprintf("loading memory map\n");
        mb2_info_mem_map_t *mmp = (mb2_info_mem_map_t *)p;

        ASSERT(mmp->type == MB2_INFO_MEMORY_MAP);
        kprintf("\tmemory map total size: %d\n", mmp->size);
        kprintf("\teach entry has a size of %d\n", mmp->entry_size);

        mb2_info_mem_map_entry_t *end =
            (mb2_info_mem_map_entry_t *)(p + mmp->size);
        mb2_info_mem_map_entry_t *ep =
            (mb2_info_mem_map_entry_t *)(p + sizeof(mb2_info_mem_map_t));

        pma_init();

        while (ep < end) {
                kprintf(
                    "\ttype = %d, base = 0x%x, size = %d\n", ep->type,
                    ep->base_addr, ep->length);

                if (ep->type == 1) {
                        _pma_mark_usable(ep->base_addr, ep->length);
                }

                p  = (byte *)ep + mmp->entry_size;
                ep = (void *)p;
        }

        _pma_mark_reserved(0, PREMAPPED_END);
}

static u64
ig_mb2_load_rsdp(byte *p)
{
        kprintf("loading RSDP\n");

        mb2_info_acpi_rsdp_t *rsdp = (void *)p;
        ASSERT(rsdp->type == MB2_INFO_ACPI_RSDP1);

        if (!kstrequ(8, "RSD PTR ", rsdp->sign)) {
                kprintf("\tinvalid sign: %sn\n", rsdp->sign, 8);
                return 0;
        }

        /* TODO: checksum */

        kprintf("\tOEM: %sn, revision: %d\n", rsdp->oemid, 6, rsdp->rev);
        kprintf("RSDT at physical location 0x%x\n", rsdp->rsdt_addr);

        return rsdp->rsdt_addr;
}

static u64
ig_mb2_load_rsdp2(byte *p)
{
        kprintf("loading XSDP\n");

        mb2_info_acpi_rsdp2_t *rsdp2 = (void *)p;
        ASSERT(rsdp2->legacy.type == MB2_INFO_ACPI_RSDP2);

        if (!kstrequ(8, "RSD PTR ", rsdp2->legacy.sign)) {
                kprintf("\tinvalid sign: %sn\n", rsdp2->legacy.sign, 8);
                return 0;
        }

        /* TODO: checksum */

        kprintf(
            "\tOEM: %sn, revision: %d\n", rsdp2->legacy.oemid, 6,
            rsdp2->legacy.rev);
        kprintf("XSDT at physical location 0x%x\n", rsdp2->xsdt_addr);

        return rsdp2->xsdt_addr;
}

static byte *
__align8(byte *p)
{
        uintptr ret = (uintptr)p;
        --ret;
        ret &= 0xFFFFFFFFFFFFFFF8;
        ret += 8;
        return (byte *)ret;
}

extern void ig_entry_bsp(u64 rsdt_addr, u64 xsdt_addr);

void
ig_mb2_c_entry(mb2_info_header_t *mb2_info_header)
{
        con_init();
        con_set_driver(vga_text_init());
        kprintf(
            "Multiboot2 header: total %d bytes at 0x%x\n",
            mb2_info_header->total_size, mb2_info_header);
        byte *p   = (byte *)mb2_info_header;
        byte *end = p + mb2_info_header->total_size;

        p += sizeof(mb2_info_header_t);

        char *cmdline = NULL;

        u64 rsdt_addr = 0;
        u64 xsdt_addr = 0;

        byte *memmap = NULL;

        /* assert no more than 256 modules */
        byte * module[256];
        byte **module_end = module;

        while (p < end) {
                mb2_info_tag_header_t *th = (mb2_info_tag_header_t *)p;

                kprintf("\ttag: type = %d, size = %d\n", th->type, th->size);

                switch (th->type) {
                case MB2_INFO_CMDLINE:
                        cmdline = (char *)(p + sizeof(mb2_info_tag_header_t));
                        kprintf("\tboot cmdline: %s\n", cmdline);
                        break;
                case MB2_INFO_MODULES:
                        VERIFY(module_end - module < 256, "too many modules");
                        *module_end++ = p;
                        break;
                case MB2_INFO_MEMORY_MAP:
                        memmap = p;
                        break;
                case MB2_INFO_ACPI_RSDP1:
                        rsdt_addr = ig_mb2_load_rsdp(p);
                        break;
                case MB2_INFO_ACPI_RSDP2:
                        xsdt_addr = ig_mb2_load_rsdp2(p);
                        break;
                default:
                        break;
                }
                p += th->size;
                p = __align8(p);
        }

        VERIFY(memmap, "multiboot2: no memory map found!");

        ig_mb2_load_mem_map(memmap);
        for (byte **i = module; i < module_end; ++i) { ig_mb2_load_module(*i); }

        ig_entry_bsp(rsdt_addr, xsdt_addr);
}
