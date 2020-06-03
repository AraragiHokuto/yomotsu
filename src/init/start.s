.section	.bss

.global	_stack_end
_stack:
.skip	16384
_stack_end:

.section	.text
.global	_start
.type	_start, @function
_start:
	movabs	$_stack_end, %rsp
	xorq	%rbp, %rbp
	pushq	%rbp

	call	main

	movq	%rax, %rdi
	call	process_exit

	# should not reach here
	xchgw	%bx, %bx
	jmp .
