# OS Runtime library makefile

SRCROOT	= $(REPO_ROOT)/osrt

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
	rt/console.c	\
	rt/_crt.c	\
	rt/crt1.s	\
	rt/memory.c	\
	rt/port.c	\
	rt/process.c	\
	rt/syscall.c	\
	rt/syscall.s

CFLAGS	+=\
	-D__RENZAN_OSRT	\
	-nostdlibinc	\
	-I$(SRCROOT)/include

.include "src.rules.make"

.MAIN: all
all: libosrt.a .PHONY
	@echo "Installing libosrt.a to ${SYSTEM_ROOT}"
	@mkdir -p ${SYSTEM_ROOT}/resource/development/library
	@cp libosrt.a ${SYSTEM_ROOT}/resource/development/library
	@cp -r $(SRCROOT)/include ${SYSTEM_ROOT}/resource/development/

libosrt.a: ${OBJS}
	@echo "\tAR\tlibosrt.a"
	@ar rcs libosrt.a ${OBJS}
