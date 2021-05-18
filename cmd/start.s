.section	.bss

.global	_stack_end
_stack:
.skip	8388608
_stack_end:

.section	.text
.global	__init_start
.type	__init_start, @function
__init_start:
	movabs	$_stack_end, %rsp
	xorq	%rbp, %rbp
	pushq	%rbp

	xchgw	%bx, %bx
	call	__init_main

	# should not reach here
	xchgw	%bx, %bx
	jmp .
