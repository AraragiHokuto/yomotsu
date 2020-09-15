.section	.text

.global	_start
_start:
	# load stack pointer, which always lies at the end of address space
	movabs	$0x7ffffffffff8, %rsp
	movq	(%rsp), %rsp

	xorq	%rbp, %rbp
	pushq	%rbp

	callq	__libc_init

	callq	__main_wrapper

	# should never reach here
	jmp .
.size _start, . - _start
