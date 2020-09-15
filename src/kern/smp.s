.set	VADDR_BASE,	0xFFFFFFFF80000000

.section	.text
.global	_ap_trampoline
.global	_ap_start
.code16

_ap_trampoline:
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
	movl	$_ap_start - VADDR_BASE, %eax
	ljmp	$0x08, $(_ap_trampoline32 - _ap_trampoline + 0x7000)

.code32
_ap_trampoline32:
	movl	$(_ap_start - VADDR_BASE), %eax
	jmp	*%eax

.code32
_ap_start:
	movw	$0x10, %ax
	movw	%ax, %ds

	# load pml4 into cr3
	movl	$_pml4 - VADDR_BASE, %eax
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

	lgdt	_gdt64_ptr32 - VADDR_BASE

	# go long mode
	jmp	$0x08, $_ap_lm_entry

.code64
_ap_lm_entry	= . - VADDR_BASE
	# load gdt in higher half
	lgdt	_gdt64_ptr

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
	movq	__ap_stack, %rsp
	addq	$16384, %rsp
	xorq	%rbp, %rbp
	pushq	%rbp
	movq	%rsp, %rbp

	# go higher half
	movabs	$_ap_higher_half_entry, %rax
	jmp	*%rax

_ap_higher_half_entry:
	xorq	%rbp, %rbp
	pushq	%rbp
	call	smp_ap_entry
_1:
	hlt
	jmp _1


