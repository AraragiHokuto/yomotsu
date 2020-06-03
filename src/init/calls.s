.section	.bss
.global	_entry_func, _entry_arg
_entry_func:
	.long	0
_entry_arg:
	.long	0

.section	.text

.global	__do_syscall
.type	__do_syscall, @function

__do_syscall:
	movq	%r8, %r9
	movq	%r9, %r10
	movq	%rdi, %rax
	movq	%rsi, %rdi
	movq	%rdx, %rsi
	movq	%rcx, %r8
	xchgq	-8(%rsp), %r12

	pushq	%rbp
	movq	%rsp, %rdx

	syscall

	movq	%rdx, %rsp
	popq	%rbp

	movq	-8(%rsp), %r12
	ret

.global	__process_spawn_entry
.type	__process_spawn_entry, @function
__process_spawn_entry:
	movq	(_entry_func), %rax
	movq	(_entry_arg), %rdi
	movabs	$_stack_end, %rsp
	xorq	%rbp, %rbp
	pushq	%rbp
	call	%rax
	jmp	.
