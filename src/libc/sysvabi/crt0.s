.section	.text

.global	_start
_start:
	movabs	$0x7ffffffffffffff8, %rbx
	movq	(%rbx), %rsp

	xorq	%rbp, %rbp
	pushq	%rbp

	callq	__libc_init

	callq	__init

	movq	-0x08(%rbx), %rdi
	movq	-0x10(%rbx), %rsi

	movq	%rax, %rdi
	callq	__exit
.size _start, . - _start
