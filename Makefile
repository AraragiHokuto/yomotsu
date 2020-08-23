AS=x86_64-elf-as
CC=clang
LD=clang

all: kernel

kernel: build/kern/kernel.bin
build/kern/kernel.bin: FORCE
	./src/kern/configure
	make -C build/kern

src/init/init: FORCE
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
