# Makefile for kernel build

## Arch-specific definitions

.include "arch/${BUILD_ARCH}/Makefile.inc"

SRCS	=\
	${ARCH_SRCS:@SRC@arch/amd64/${SRC}@}	\
	acpi.c	\
	console.c	\
	descriptor.c	\
	futex.c	\
	init_loader.c	\
	init_loader.s	\
	interrupt.c	\
	interrupt.s	\
	kernel.c	\
	kobject.c	\
	memory.c	\
	mutex.c	\
	panic.c	\
	port.c	\
	port_ifce.c	\
	process.c	\
	process.s	\
	sched.c	\
	sched.s	\
	simd.c	\
	smp.c	\
	smp.s	\
	spinlock.c	\
	syscall.c	\
	syscall.s	\
	string.c	\
	timer.c	\
	user_memory.c	\
	vga_text.c

CFLAGS	+=\
	${ARCH_CFLAGS}	\
	-D_KERNEL	\
	-ffreestanding	\
	-fno-stack-protector	\
	-nostdlib	\
	-nostdinc

.include "src.rules.make"

LDFLAGS	+=\
	${ARCH_LDFLAGS}

.MAIN: all
all: kernel.bin .PHONY
	@echo "Installing kernel to ${SYSTEM_ROOT}"
	@mkdir -p ${SYSTEM_ROOT}/boot
	@cp kernel.bin ${SYSTEM_ROOT}/boot/kernel.bin

kernel.bin: $(OBJS)
	@echo "\tLD\tkernel.bin"
	@$(LD) $(LDFLAGS) -o kernel.bin $(OBJS)

