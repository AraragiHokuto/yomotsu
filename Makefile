AS=x86_64-elf-as
CC=clang
LD=clang

all: kernel libc init

kernel: build/kern/kernel.bin
libc: build/libc/liborihime.a
init: src/init/init

build/kern/kernel.bin: FORCE
	./src/kern/configure
	make -C build/kern

build/libc/liborihime.a: FORCE
	./src/libc/configure
	make -C build/libc

src/init/init: build/libc/liborihime.a FORCE
	make -C src/init

iso: orihime.iso
orihime.iso: build/kern/kernel.bin src/init/init
	mkdir -p isodir/boot/grub/
	cp grub.cfg isodir/boot/grub/
	cp build/kern/kernel.bin build/kern/srcmap.bin isodir/boot/
	cp src/init/init isodir/boot/
	grub-mkrescue -o orihime.iso isodir/

runbochs: orihime.iso
	bochs -q

clean: FORCE
	rm -rf build
	make -C src/init clean
FORCE:
