AS=$(LLVM_PREFIX)/bin/clang
CC=$(LLVM_PREFIX)/bin/clang
LD=$(LLVM_PREFIX)/bin/clang
CFLAGS=--target=x86_64-pc-orihime-elf -Wall -Wextra -std=gnu99 -I../ -I../libc -I../libc/cstd -static -nostdlib -nostdlibinc -fno-pie -g -Wno-unused-function -Wno-sign-compare
LDFLAGS=--target=x86_64-pc-orihime-elf -nostdlib -fno-pie -static -L../../build/libc -lorihime

init: stub_proc_bin.o init.c.o start.s.o ../../build/libc/liborihime.a
	$(LD) -Wl,-Tlinker.ld -o $@ stub_proc_bin.o init.c.o start.s.o $(LDFLAGS)

stub_proc_bin.o: stub_proc.c ../../build/libc/liborihime.a
	$(CC) $(CFLAGS) -Wl,-Tstubproc_linker.ld -ostub_proc_bin stub_proc.c $(LDFLAGS)
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 stub_proc_bin stub_proc_bin.o

start.s.o: start.s
	$(AS) -c --target=x86_64-pc-orihime-elf start.s -ostart.s.o

init.c.o: init.c
	$(CC) $(CFLAGS) -c -oinit.c.o init.c

clean: FORCE
	rm -rf *.o init stub_proc_bin
FORCE:
