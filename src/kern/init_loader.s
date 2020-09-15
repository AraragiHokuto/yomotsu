.global	__init_start
.type	__init_start, @function

__init_start:
	cli
	pushfq
	popq	%r11
	orq	$(1 << 9), %r11

	movq	%rdi, %rcx

	swapgs
	sysretq
