# Toplevel Makefileo

SUBDIRS	=\
	os

.include "subdir.rules.make"

orihime.iso: all
	mkdir -p ${SYSTEM_ROOT}/boot/grub/
	cp grub.cfg ${SYSTEM_ROOT}/boot/grub/
	grub-mkrescue -o orihime.iso ${SYSTEM_ROOT}

iso: orihime.iso

runbochs: orihime.iso .EXEC
	bochs -q

runqemu: orihime.iso .EXEC
	qemu-system-x86_64 -cdrom orihime.iso -cpu Skylake-Client -smp 4 -m 1G
