AS=as
CC=clang
LD=clang
CFLAGS=-target x86_64-pc-none-elf -ffreestanding -Wall -Wextra -std=gnu99 -I./ -static -nostdlib -fno-pie -mno-implicit-float -g -mno-red-zone -Wno-unused-function -Wno-sign-compare -fno-omit-frame-pointer -I../
LDFLAGS=-target x86_64-pc-none-elf -ffreestanding -nostdlib -z max-page-size=0x1000 -fno-pie -static -mcmodel=kernel

OBJS=$(patsubst %.s,%_s.o,$(wildcard *.s)) $(patsubst %.c,%_c.o,$(wildcard *.c))

init: $(OBJS)
	$(LD) $(LDFLAGS) -Wl,-Tlinker.ld -o $@ $(OBJS)

%_c.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%_s.o: %.s
	$(AS) -o $@ $<

clean: FORCE
	rm -rf *.o init
FORCE:
