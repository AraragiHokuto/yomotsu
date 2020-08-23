#include <kern/acpi.h>
#include <kern/boolean.h>
#include <kern/console.h>
#include <kern/memory.h>
#include <kern/string.h>

static acpi_sdt_header_t *madt = NULL;

static boolean
verify_checksum(acpi_sdt_header_t *table)
{
        byte *t = (byte *)(table);

        u8 sum = 0;
        for (uint i = 0; i < table->length; ++i) { sum += t[i]; }

        return sum == 0;
}

#define HASH_SIGN(c1, c2, c3, c4) \
        (((u32)c1) | ((u32)c2 << 8) | ((u32)c3 << 16) | ((u32)c4 << 24))

static u32
hash_sign(acpi_sdt_header_t *table)
{
        return HASH_SIGN(
            table->sign[0], table->sign[1], table->sign[2], table->sign[3]);
}

static void
acpi_load_madt(acpi_sdt_header_t *sdt)
{
        kprintf("\tloading MADT from 0x%x\n", sdt);
        madt = kmem_alloc(sdt->length);
        if (!madt) return;

        kmemcpy(madt, sdt, sdt->length);
}

static void
acpi_load_sdt(uintptr sdt_pma)
{
        acpi_sdt_header_t *sdt = vm_get_vma_for_pma(sdt_pma);

        if (!sdt) { kprintf("\tSDT mapping failed at 0x%x\n", sdt_pma); }

        sdt = vm_get_vma_for_pma(sdt_pma);

        switch (hash_sign(sdt)) {
        case HASH_SIGN('A', 'P', 'I', 'C'):
                acpi_load_madt(sdt);
                break;
        default:
                kprintf("\tunhandled signature %sn; ignored\n", sdt->sign, 4);
                break;
        }
}

void
acpi_load_rsdt(uintptr rsdt_pma)
{
        acpi_rsdt_t *rsdt = vm_get_vma_for_pma(rsdt_pma);
        if (!rsdt) {
                kprintf("RSDT mapping failed\n");
                return;
        }

        size_t rsdt_size = rsdt->h.length;
        kprintf("\tRSDT: mapped at 0x%x, size %d\n", rsdt, rsdt_size);

        if (!verify_checksum(&rsdt->h)) {
                kprintf("\tRSDT: checksum failed\n");
                return;
        }

        uint entries = (rsdt_size - sizeof(acpi_rsdt_t)) / sizeof(u32);

        for (uint i = 0; i < entries; ++i) {
                kprintf("\tRSDT: loading %d of %d\n", i + 1, entries);
                acpi_load_sdt(rsdt->sdt_ptrs[i]);
        }
}

void
acpi_load_xsdt(uintptr xsdt_pma)
{
        acpi_xsdt_t *xsdt = vm_get_vma_for_pma(xsdt_pma);
        if (!xsdt) {
                kprintf("XSDT mapping failed\n");
                return;
        }

        size_t xsdt_size = xsdt->h.length;
        kprintf("\tXSDT: mapped at 0x%x, size %d\n", xsdt, xsdt_size);

        if (!verify_checksum(&xsdt->h)) {
                kprintf("\tXSDT: checksum failed\n");
                return;
        }

        uint entries = (xsdt_size - sizeof(acpi_xsdt_t)) / sizeof(u64);

        for (uint i = 0; i < entries; ++i) {
                kprintf("\tXSDT: loading %d of %d\n", i + 1, entries);
                acpi_load_sdt(xsdt->sdt_ptrs[i]);
        }
}

acpi_madt_lapic_t *
acpi_madt_lapic_info(void)
{
        if (!madt) { return NULL; }

        byte *p = (void *)madt;
        p += sizeof(acpi_sdt_header_t);
        return (void *)p;
}

acpi_madt_record_t *
acpi_madt_first_record(void)
{
        if (!madt) { return NULL; }

        byte *p = (void *)madt;
        p += sizeof(acpi_sdt_header_t);
        p += sizeof(acpi_madt_lapic_t);
        return (void *)p;
}

acpi_madt_record_t *
acpi_madt_next_record(acpi_madt_record_t *this)
{
        byte *end = (byte *)madt + madt->length;
        byte *p   = (void *)this;

        VERIFY((p > (byte *)madt) && (p <= end), "invalid this pointer");
        p += this->header.length;
        return p < end ? (void *)p : NULL;
}
