#ifndef IZANAMI_DESCRIPTOR_H__
#define IZANAMI_DESCRIPTOR_H__

#include <kern/macrodef.h>
#include <kern/types.h>

#ifdef _KERNEL

struct gdt_code_desc_s {
	u64	ign_1	:42,
		c	:1,
		mb3	:2,
		dpl	:2,
		p	:1,
		ign_2	:5,
		l	:1,
		d	:1,
		ign_3	:9;
} PACKED;

typedef struct gdt_code_desc_s	gdt_code_desc_t;

struct gdt_data_desc_s {
	u64	ign_1	:41,
		w	:1,
		e	:1,
		mbz	:1,
		mb1	:1,
		dpl	:2,
		p	:1,
		ign_2	:16;
} PACKED;

typedef struct gdt_data_desc_s	gdt_data_desc_t;

struct gdt_tss_selector_s {
	u64	limit_l	:16,
		base_l1	:24,
		type	:4,
		mbz_1	:1,
		dpl	:2,
		p	:1,
		limit_h	:4,
		avl	:1,
		ign_1	:2,
		g	:1,
		base_l2	:8;
	u32	base_h;
	u32	mbz_2;
} PACKED;

typedef struct gdt_tss_selector_s	gdt_tss_selector_t;

struct gdt_s {
	u64	null_desc;
	gdt_code_desc_t	code_kern;
	gdt_data_desc_t	data_kern;
	gdt_data_desc_t	data_user;
	gdt_code_desc_t	code_user;
	gdt_tss_selector_t	tss;
} PACKED;

typedef struct gdt_s	gdt_t;

struct gdtr_s {
	u16	limit;
	void	*addr;
} PACKED;

typedef struct gdtr_s	gdtr_t;

struct tss_s {
	u32	resv_1;
	u64	rsp0;
	u64	rsp1;
	u64	rsp2;
	u64	resv_2;
	u64	ist1;
	u64	ist2;
	u64	ist3;
	u64	ist4;
	u64	ist5;
	u64	ist6;
	u64	ist7;
	u64	resv_3;
	u16	resv_4;
	u16	iopb;
} PACKED;

typedef struct tss_s	tss_t;

void	descriptor_init(void);
void	descriptor_load_tss(tss_t *tss);

#endif	/* _KERNEL */

#endif	/* ORIHIME_DESCRIPTOR_H__ */
