.set	VADDR_BASE,	0xFFFFFFFF80000000

.section	.text
.global	ig_ap_trampoline
.global	ig_ap_start
.code16

# 16bit AP start-up trampoline code
# This will be triggered by SIPI
ig_ap_trampoline:
	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss

	# enable protected mode
	lgdt	0x6fde
	movl	%cr0, %eax
	orl	$1, %eax
	movl	%eax, %cr0

	# go to ap startup code
	movl	$ig_ap_start - VADDR_BASE, %eax
	ljmp	$0x08, $(ig_ap_trampoline32 - ig_ap_trampoline + 0x7000)

# 32bit AP start-up trampoline code
.code32
ig_ap_trampoline32:
	movl	$(ig_ap_start - VADDR_BASE), %eax
	jmp	*%eax

.code32
ig_ap_start:
	movw	$0x10, %ax
	movw	%ax, %ds

	# load pml4 into cr3
	movl	$ig_pml4 - VADDR_BASE, %eax
	movl	%eax, %cr3

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

	popl	%edi

	lgdt	ig_gdt64_ptr32 - VADDR_BASE

	# go long mode
	jmp	$0x08, $ig_ap_lm_entry

.code64
ig_ap_lm_entry	= . - VADDR_BASE
	# load gdt in higher half
	lgdt	ig_gdt64_ptr

	# load 64-bit data segments
	movw	$0x10, %ax
	movw	%ax, %ds

	xorq	%rax, %rax
	movq	%rax, %es
	movq	%rax, %fs
	movq	%rax, %fs
	movq	%rax, %gs
	movq	%rax, %ss

	# setup c stack
	movq	ig_ap_stack, %rsp
	addq	$16384, %rsp
	xorq	%rbp, %rbp
	pushq	%rbp
	movq	%rsp, %rbp

	# go higher half
	movabs	$ig_ap_higher_half_entry, %rax
	jmp	*%rax

ig_ap_higher_half_entry:
	xorq	%rbp, %rbp
	pushq	%rbp
	call	ig_smp_ap_entry
_1:
	hlt
	jmp _1


