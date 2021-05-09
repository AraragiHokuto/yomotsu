# -*- make -*-

# AMD64 arch-specific definitions

SRCS.amd64	=\
	arch/amd64/ig_mboot2.s	\
	arch/amd64/ig_mboot2.c	\
	arch/amd64/ig_smp.s	\
	arch/amd64/ig_smp.c	\
	arch/amd64/hal_smp.c

CFLAGS.amd64	=\
	-mcmodel=kernel	\
	-fno-pie	\
	-mno-implicit-float	\
	-mno-red-zone

LDFLAGS.amd64	=\
	-zmax-page-size=0x1000	\
	-T${SRCROOT}/arch/amd64/linker.ld
