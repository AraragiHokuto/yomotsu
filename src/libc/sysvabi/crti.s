.section	.init
.global	__init
__init:
	pushq	%rbp
	movq	%rsp, %rbp

.section	.fini
.global	__fini
__fini:
	pushq	%rbp
	movq	%rsp, %rbp
