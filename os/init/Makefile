# Init process makefile

SRCS	=\
	init.c	\
	start.s	\
	stub_proc.c

.include "src.rules.make"

.MAIN: all
all: init .PHONY
	@echo "Installing init to ${SYSTEM_ROOT}"
	@mkdir -p ${SYSTEM_ROOT}/boot
	@cp init ${SYSTEM_ROOT}/boot

init: stub_proc_bin.o init.c.o start.s.o
	@echo "\tLD\tinit"
	@$(CC) $(CFLAGS) -Wl,-T${REPO_ROOT}/os/init/linker.ld -o $@ stub_proc_bin.o init.c.o start.s.o 

stub_proc_bin.o: stub_proc.o
	@$(CC) $(CFLAGS) -Wl,-T${REPO_ROOT}/os/init/stubproc_linker.ld -ostub_proc_bin stub_proc.o
	@echo "\tOBJCOPY\tstub_proc_bin.o"
	@objcopy -I binary -O elf64-x86-64 -B i386:x86-64 stub_proc_bin stub_proc_bin.o
