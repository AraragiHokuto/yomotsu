# ig_mboot2.s -- Multiboot2 entry for igniter on AMD64


.section	.multiboot2
.align	8

ig_mb2h_begin:
# Multiboot2 magic fields
.set	MB2H_MAGIC,	0xE85250D6
.set	MB2H_ARCH,	0
.set	MB2H_LEN,	ig_mb2h_end - ig_mb2h_begin
.set	MB2H_CKSUM,	-(MB2H_MAGIC + MB2H_ARCH + MB2H_LEN)

.long	MB2H_MAGIC
.long	MB2H_ARCH
.long	MB2H_LEN
.long	MB2H_CKSUM

# Multiboot2 address tag
.set	MB2H_ADDR,		_mb2h_off
.set	LOADER_ADDR,		_loader_off
.set	LOADER_END_ADDR,	_loader_end_off
.set	BSS_END_ADDR,		_bss_end_off

.align	8
.short	2		# type
.short	0		# flags
.long	24		# size
.long	MB2H_ADDR
.long	LOADER_ADDR
.long	LOADER_END_ADDR
# .long	BSS_END_ADDR
.long	0x2000000	# mark all of first 32MB as bss
			# to avoid corrupting other data

# Multiboot2 entry address tag
.set	VADDR_BASE,	0xFFFFFFFF80000000

.align	8
.short	3				# type
.short	0				# flags
.long	12				# size
.long	ig_mb2_entry - VADDR_BASE	# entry addr

# Multiboot2 info request tag
.align	8
.short	1	# type
.short	0	# flags
.long	16	# size
.long	14	# ACPI old RSDP
.long	15	# ACPI new RSDP

# Multiboot2 frambuffer tag
# .align	8
# .short	5	# type
# .short	0	# flags
# .long	20	# size
# .long	0	# height
# .long	0	# width
# .long	32	# depth

# Multiboot2 end tag
.align	8
.short	0	# type
.short	0	# flags
.long	8	# size

ig_mb2h_end:

.section	.bss
.align	16
ig_mb2_boot_stack_bottom:
.skip	16384		# 16K boot stack
ig_mb2_boot_stack_top:

.set	ESP_ADDR,	ig_mb2_boot_stack_top - VADDR_BASE

.section	.data
# initial page table
.macro	pml4e addr
	.quad	((\addr - VADDR_BASE) + 3)
.endm

.macro	pdpe addr
	.quad	((\addr - VADDR_BASE) + 3)
.endm

.macro	pde addr
	.quad	(\addr + 131)
.endm

.align	4096
.global ig_pml4	# for smp.s
ig_pml4:
pml4e	ig_pdp
.skip	2040
pml4e	ig_pml4
.skip	2032
pml4e	ig_pdp

ig_pdp:
pdpe	ig_pd_kern1
.skip	4072
pdpe	ig_pd_kern1
.quad	ig_pd_kern2

ig_pd_kern1:
# first 32M
pde	0
pde	0x200000
pde	0x400000
pde	0x600000
pde	0x800000
pde	0xA00000
pde	0xC00000
pde	0xE00000
pde	0x1000000
pde	0x1200000
pde	0x1600000
pde	0x1800000
pde	0x1A00000
pde	0x1C00000
pde	0x1E00000
pde	0x2000000
.skip	3968

ig_pd_kern2:
.skip	4096

.align	16
ig_gdt64:
# null descriptor
.short	0xFFFF
.short	0
.byte	0
.byte	0
.byte	1
.byte	0
# code descriptor: kernel
.short	0
.short	0
.byte	0
.byte	0x9A
.byte	0xAF
.byte	0
# data descriptor: kernel
.short	0
.short	0
.byte	0
.byte	0x92
.byte	0
.byte	0

ig_gdt64_len	= . - ig_gdt64

.global	ig_gdt64_ptr32, ig_gdt64_ptr	# for smp.s
ig_gdt64_ptr32:
.short	ig_gdt64_len
.quad	ig_gdt64	- VADDR_BASE
ig_gdt64_ptr:
.short	ig_gdt64_len
.quad	ig_gdt64

.section	.rodata
.align	16
ig_panic_mb2_state_str:
	.ascii	"panic: invalid multiboot2 state"
ig_panic_mb2_state_len	= . - ig_panic_mb2_state_str

ig_panic_no_lm_str:
	.ascii	"panic: long mode unavaliable"
ig_panic_no_lm_len	= . - ig_panic_no_lm_str

ig_panic_no_cpuid_str:
	.ascii	"panic: CPUID unavaliable"
ig_panic_no_cpuid_len	= . - ig_panic_no_cpuid_str

.section	.text
.code32
.global	ig_mb2_entry
.type	ig_mb2_entry, @function
ig_mb2_entry:
	xchgw	%bx, %bx
	cli
	movl	$ESP_ADDR, %esp

	# check for multiboot2 state
.set	MB2_STATE_MAGIC, 0x36D76289
	xorl	$MB2_STATE_MAGIC, %eax
	jnz	ig_panic_mb2_state

	# preserve multiboot2 boot info
	pushl	%ebx

	# check if CPUID is avaliable
	pushfl
	popl	%eax
	movl	%eax, %ecx

	xorl	$(1 << 21), %eax
	pushl	%eax
	popfl

	pushfl
	popl	%eax

	pushl	%ecx
	popfl

	xorl	%eax, %ecx
	jz	ig_panic_no_cpuid

	# check if long mode is avaliable
	movl	$0x80000000, %eax
	cpuid
	cmpl	$0x80000001, %eax
	jb	ig_panic_no_longmode

	movl	$0x80000001, %eax
	cpuid
	testl	$(1 << 29), %edx
	jz	ig_panic_no_longmode

	# load pml4 into cr3
	movl	$ig_pml4 - VADDR_BASE, %edi
	movl	%edi, %cr3

	# enable PAE
	movl	%cr4, %eax
	orl	$(1 << 5), %eax
	movl	%eax, %cr4

	# set LM-bit
	movl	$0xC0000080, %ecx
	rdmsr
	orl	$(1 << 8), %eax
	wrmsr

	# enable paging
	movl	%cr0, %eax
	orl	$(1 << 31), %eax
	movl	%eax, %cr0

	# we won't have popl after long mode activated. restore boot info now.
	popl	%edi

	# load 64bit GDT
	lgdt	ig_gdt64_ptr32 - VADDR_BASE

	# go long mode
	jmp	$0x08, $ig_lm_entry
.code64
ig_lm_entry	= . - VADDR_BASE
	# load gdt in higher half
	lgdt	ig_gdt64_ptr

	# load 64-bit data segments
	movw	$0x10, %ax
	movw	%ax, %ds

	# clear other segment registers
	xorq	%rax, %rax
	movq	%rax, %es
	movq	%rax, %fs
	movq	%rax, %gs
	movq	%rax, %ss

	# setup c stack in higher half
	addq	$VADDR_BASE, %rsp

	# load gs base addr for BSP
	movabs	$bsp_percpu_data, %rax
	movq	%rax, %rdx
	shrq	$32, %rdx
	movl	%eax, %eax
	movabs	$0xC0000101, %rcx
	wrmsr
	movabs	$0xC0000102, %rcx
	wrmsr
	movabs	$bsp_percpu_data, %rax
	movq	%rax, (%rax)

	# go higher half
	movabs	$ig_higher_half_entry, %rax
	jmp	*%rax

ig_higher_half_entry:
	xorq	%rbp, %rbp
	pushq	%rbp
	call	ig_mb2_c_entry
1:
	cli
	hlt
	jmp	1b

.code32
ig_panic_mb2_state:
	movl	$ig_panic_mb2_state_str - VADDR_BASE, %ecx
	movl	$ig_panic_mb2_state_len, %edx
	jmp	ig_panic32

ig_panic_no_cpuid:
	movl	$ig_panic_no_cpuid_str - VADDR_BASE, %ecx
	movl	$ig_panic_no_cpuid_len, %edx
	jmp	ig_panic32

ig_panic_no_longmode:
	movl	$ig_panic_no_lm_str - VADDR_BASE, %ecx
	movl	$ig_panic_no_lm_len, %edx
	jmp	ig_panic32

ig_puts32:
	pushl	%ebp
	pushl	%ebx
	movl	$0xB8000, %ebp
	xorl	%ebx, %ebx
1:
	movb	(%ecx, %ebx), %al
	movb	%al, (%ebp, %ebx, 2)
	movb	$0x0F, 1(%ebp, %ebx, 2)
	incl	%ebx
	cmpl	%ebx, %edx
	jne	1b

	popl	%ebx
	popl	%ebp
	ret

ig_panic32:
	cli
	call	ig_puts32
	xchgw	%bx, %bx	# bochs magic break
1:
	hlt
	jmp 1b
