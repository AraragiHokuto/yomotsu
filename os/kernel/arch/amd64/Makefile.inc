# -*- make -*-

# AMD64 arch-specific definitions

ARCH_SRCS	=\
	boot/mboot2.s	\
	boot/mboot2.c

ARCH_CFLAGS	=\
	-mcmodel=kernel	\
	-fno-pie	\
	-mno-implicit-float	\
	-mno-red-zone

ARCH_MAKEDIRS	=\
	boot

LINKER_SCRIPT	:= ${.PARSEDIR}/linker.ld
ARCH_LDFLAGS	=\
	-zmax-page-size=0x1000	\
	-T${REPO_ROOT}/os/kernel/arch/amd64/linker.ld
