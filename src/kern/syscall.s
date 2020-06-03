.section	.text
.global	__syscall_entry
.type	__syscall_entry, @function

__syscall_entry:
	swapgs

	# load kernel stack
	movq	%gs:0x28, %rsp
	xorq	%rbp, %rbp
	pushq	%rbp

	pushq	%rcx
	pushq	%rdx
	pushq	%r11

	movq	%r8, %rdx
	movq	%r9, %rcx
	movq	%r10, %r8
	movq	%r12, %r9

	movq	__syscall_count, %r10
	cmpq	%r10, %rax
	jnl	__syscall_entry_invalid

	# load syscall address
	movabs	$__syscall_table, %r10
	movq	(%r10, %rax, 8), %r10
	sti
	callq	*%r10
	cli
	jmp	__syscall_entry_valid

__syscall_entry_invalid:
	movq	(__syscall_invil_error), %rax

__syscall_entry_valid:
	pushq	%rax
	call	__process_on_sysret
	popq	%rax

	popq	%r11
	popq	%rdx
	popq	%rcx

	swapgs
	sysretq

.global	__reincarnate_return
.type	__reincarnate_return, @function
__reincarnate_return:
	cli
	pushfq
	pushq	%rdi

	call	__process_on_sysret

	popq	%rcx
	popq	%r11
	orq	$(1 << 9), %r11

	swapgs
	sysretq

.global	__process_spawn_start
.type	__process_spawn_start, @function
__process_spawn_start:
	cli
	pushfq
	pushq	%rdi

	call	__process_on_sysret

	popq	%rcx
	popq	%r11
	orq	$(1 << 9), %r11

	swapgs
	sysretq
