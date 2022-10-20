# Makefile for kernel build

## Arch-specific definitions

SRCROOT	= ${REPO_ROOT}/kernel

.include "arch/Makefile.inc"

SRCS	=\
	ig_entry.c	\
	k_acpi.c	\
	k_console.c	\
	k_descriptor.c	\
	k_futex.c	\
	ig_init_loader.c	\
	k_int.c	\
	k_kobj.c	\
	k_memory.c	\
	k_mutex.c	\
	k_panic.c	\
	k_port.c	\
	k_port_ifce.c	\
	k_thread.c	\
	k_sched.c	\
	k_simd.c	\
	k_spinlock.c	\
	k_syscall.c	\
	k_string.c	\
	k_timer.c	\
	k_user_mem.c	\
	${SRCS.${BUILD_ARCH}}

CFLAGS	+=\
	-I${SRCROOT}	\
	-D_KERNEL	\
	-D__RZ_KERNEL	\
	-ffreestanding	\
	-fno-stack-protector	\
	-nostdlib	\
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

