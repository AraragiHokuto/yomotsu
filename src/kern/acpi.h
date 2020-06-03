#ifndef	IZANAMI_ACPI_H_
#define	IZANAMI_ACPI_H_

#include <kern/types.h>
#include <kern/macrodef.h>

#ifdef _KERNEL

typedef struct acpi_sdt_header_s {
	char	sign[4];
	u32	length;
	u8	rev;
	u8	checksum;
	char	oem_id[6];
	char	oem_table_id[8];
	u32	oem_rev;
	u32	creator_id;
	u32	creator_rev;
} PACKED acpi_sdt_header_t;

typedef struct acpi_rsdt_s {
	acpi_sdt_header_t h;
	u32	sdt_ptrs[];
} PACKED acpi_rsdt_t;

typedef struct acpi_xsdt_s {
	acpi_sdt_header_t h;
	u64	sdt_ptrs[];
} PACKED acpi_xsdt_t;

/* === MADT typedefs === */

enum ACPI_MADT_RECORD_TYPE {
	ACPI_MADT_RECORD_LAPIC			= 0,
	ACPI_MADT_RECORD_IOAPIC			= 1,
	ACPI_MADT_RECORD_INT_SRC_OVERRIDE	= 2,
	ACPI_MADT_RECORD_NON_MASKABLE_INT	= 4,
	ACPI_MADT_RECORD_LAPIC_ADDR_OVERRIDE	= 5
};

typedef struct acpi_madt_lapic_s {
	u32	lapic_addr;
	u32	flags;
} PACKED acpi_madt_lapic_t;

typedef struct acpi_madt_record_header_s {
	u8	type;
	u8	length;
} PACKED acpi_madt_record_header_t;

typedef struct acpi_madt_record_lapic_s {
	acpi_madt_record_header_t	header;
	u8	processor_id;
	u8	apic_id;
	u32	flags;
} PACKED acpi_madt_record_lapic_t;

typedef struct acpi_madt_record_ioapic_s {
	acpi_madt_record_header_t	header;
	u8	ioapic_id;
	u8	reserved;
	u32	ioapic_addr;
	u32	interrupt_base;
} PACKED acpi_madt_record_ioapic_t;

#define	ACPI_MADT_FLAG_ACTIVE_LOW	2
#define	ACPI_MADT_FLAG_LEVEL_TRIGGERED	8

typedef struct acpi_madt_record_int_src_override_s {
	acpi_madt_record_header_t	header;
	u8	bus_src;
	u8	irq_src;
	u32	interrupt;
	u16	flags;
} PACKED acpi_madt_record_int_src_override_t;

typedef struct acpi_madt_record_non_maskable_int_s {
	acpi_madt_record_header_t	header;
	u8	processor_id;
	u16	flags;
	u8	lint;
} PACKED acpi_madt_record_non_maskable_int_t;

typedef struct acpi_madt_record_lapic_addr_override_s {
	acpi_madt_record_header_t	header;
	u16	resv;
	u64	lapic_pma;
} PACKED acpi_madt_record_lapic_addr_override_t;

typedef union acpi_madt_record_u {
	acpi_madt_record_header_t		header;
	acpi_madt_record_lapic_t		lapic;
	acpi_madt_record_ioapic_t		ioapic;
	acpi_madt_record_int_src_override_t	int_src_override;
	acpi_madt_record_non_maskable_int_t	non_maskable_int;
	acpi_madt_record_lapic_addr_override_t	lapic_addr_override;
} PACKED acpi_madt_record_t;

void	acpi_load_rsdt(uintptr rsdt_pma);
void	acpi_load_xsdt(uintptr xsdt_pma);

/* === MADT === */
acpi_madt_lapic_t *	acpi_madt_lapic_info(void);
acpi_madt_record_t *	acpi_madt_first_record(void);
acpi_madt_record_t *	acpi_madt_next_record(acpi_madt_record_t *this);

#endif	/* _KERNEL */

#endif	/* IZANAMI_ACPI_H_ */
