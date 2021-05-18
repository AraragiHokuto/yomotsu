.section	.text
.global	__do_syscall
.type	__do_syscall, @function

__do_syscall:
	movq	%r9, %r10
	movq	%r8, %r9
	movq	%rdi, %rax
	movq	%rsi, %rdi
	movq	%rdx, %rsi
	movq	%rcx, %r8

	xchgq	0x08(%rsp), %r12

	pushq	%rbx
	pushq	%rbp
	pushq	%r13
	pushq	%r14
	pushq	%r15

	movq	%rsp, %rdx

	syscall

	movq	%rdx, %rsp

	popq	%r15
	popq	%r14
	popq	%r13
	popq	%rbp
	popq	%rbx

	movq	0x08(%rsp), %r12
	ret

.size	__do_syscall,	. - __do_syscall
