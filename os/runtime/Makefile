# OS Runtime library makefile

SRCS	=\
	c/assert.c	\
	c/ctype.c	\
	c/dlmalloc.c	\
	c/errno.c	\
	c/signal.c	\
	c/stdio.c	\
	c/stdlib.c	\
	c/string.c	\
	c/vfprintf.c	\
	osrt/console.c	\
	osrt/_crt.c	\
	osrt/crt1.s	\
	osrt/memory.c	\
	osrt/port.c	\
	osrt/process.c	\
	osrt/syscall.c	\
	osrt/syscall.s

CFLAGS	+=\
	-D__ORIHIME_OSRT	\
	-nostdlibinc

.include "src.rules.make"

.MAIN: all
all: libosrt.a .PHONY
	@echo "Installing libosrt.a to ${SYSTEM_ROOT}"
	@mkdir -p ${SYSTEM_ROOT}/resource/development/library
	@cp libosrt.a ${SYSTEM_ROOT}/resource/development/library

libosrt.a: ${OBJS}
	@echo "\tAR\tlibosrt.a"
	@ar rcs libosrt.a ${OBJS}
