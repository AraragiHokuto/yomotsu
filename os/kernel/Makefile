# Makefile for kernel build

## Arch-specific definitions

SRCROOT	= ${REPO_ROOT}/os/kernel

.include "arch/Makefile.inc"

SRCS	=\
	ig_entry.c	\
	k_cap.c	\
	acpi.c	\
	console.c	\
	descriptor.c	\
	futex.c	\
	init_loader.c	\
	init_loader.s	\
	interrupt.c	\
	interrupt.s	\
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
	spinlock.c	\
	syscall.c	\
	syscall.s	\
	string.c	\
	timer.c	\
	user_memory.c	\
	vga_text.c	\
	${SRCS.${BUILD_ARCH}}

CFLAGS	+=\
	-I${REPO_ROOT}/os/runtime/include \
	-I${SRCROOT}	\
	-D_KERNEL	\
	-D__RZ_KERNEL	\
	-ffreestanding	\
	-fno-stack-protector	\
	-nostdlib	\
	-nostdinc	\
	${CFLAGS.${BUILD_ARCH}}	\

.include "src.rules.make"

LDFLAGS	+=\
	${LDFLAGS.${BUILD_ARCH}}

.MAIN: all
all: kernel.bin .PHONY
	@echo "Installing kernel to ${SYSTEM_ROOT}"
	@mkdir -p ${SYSTEM_ROOT}/boot
	@cp kernel.bin ${SYSTEM_ROOT}/boot/kernel.bin

kernel.bin: $(OBJS)
	@echo -e "\tLD\tkernel.bin"
	@$(LD) $(LDFLAGS) -o kernel.bin $(OBJS)

