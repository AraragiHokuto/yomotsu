.section	.text

.global	__thread_do_switch
.type	__thread_do_switch, @function

__thread_do_switch:
	pushq	%rbx
	pushq	%rbp
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15

	pushq	%r8

	# switch stack
	movq	%rsp, (%rdi)
	movq	%rdx, %rsp

	movabs	$_1, %rax
	movq	%rax, (%rsi)

	pushq	%rcx
	ret
_1:
	popq	%rdi

	call	__thread_load_context

	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%rbp
	popq	%rbx

	ret

.global	__thread_do_start
.type	__thread_do_start, @function
__thread_do_start:
	popq	%rax

	movq	(%rdi), %r9
	xchg	%r9, %rsp

	pushq	%rdx
	pushq	%rcx
	pushq	%r8

	movabs	$_2, %rdx
	movq	%rdx, (%rsi)

	xchg	%r9, %rsp
	movq	%r9, (%rdi)

	pushq	%rax
	ret

_2:
	popq	%rdi
	call	__thread_load_context

	popq	%rdi
	ret
