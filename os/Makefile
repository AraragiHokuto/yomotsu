# OS makefile

SUBDIRS	=\
	kernel	\
	runtime	\
	init

init: runtime
runtime: kernel

.include "subdir.rules.make"
