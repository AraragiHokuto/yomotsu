# Toplevel Makefileo

SUBDIRS	=\
	os

.include "subdir.rules.make"

renzan.iso: all
	mkdir -p ${SYSTEM_ROOT}/boot/grub/
	cp grub.cfg ${SYSTEM_ROOT}/boot/grub/
	grub2-mkrescue -o renzan.iso ${SYSTEM_ROOT}

iso: renzan.iso

runbochs: renzan.iso .EXEC
	bochs -q

runqemu: renzan.iso .EXEC
	qemu-system-x86_64 -cdrom renzan.iso -cpu Skylake-Client -smp 4 -m 1G
