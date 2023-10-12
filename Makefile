QEMU = qemu-system-aarch64

K = kernel

OBJS = \
	$K/entry.o \
	$K/start.o \
	$K/uart.o \
	$K/kpgdir.o \
	$K/console.o \
	$K/string.o

TOOLPREFIX = aarch64-linux-gnu-

CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
AS = $(TOOLPREFIX)as
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

RM = rm -f

CFLAGS = -Wall -Werror -g -fno-pie -fno-pic \
		 -mcmodel=large -march=armv8-a -mtune=cortex-a72 \
		 -fno-stack-protector -static -fno-builtin \
		 -nostdlib -ffreestanding -nostartfiles \
		 -mgeneral-regs-only -MMD -MP -Iinc
LDFLAGS = -L. -z max-page-size=4096
ASFLAGS = -march=armv8-a

.PHONY: all

a: fs.img

$K/kernel.elf: $(OBJS) Makefile $K/kernel.ld
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $@ $(OBJS)
	$(OBJDUMP) -S -D $@ > $K/kernel.asm
	$(OBJDUMP) -x $@ > $K/kernel.hdr

fs.img: $K/kernel.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# .PRECIOUS: %.o

r: fs.img
	$(QEMU) -machine virt -cpu cortex-a72 -m 128 -kernel $< -nographic

hh: fs.img
	$(QEMU) -machine virt -cpu cortex-a72 -m 128 -kernel $< -nographic -monitor telnet:127.0.0.1:9191,server,nowait

gg: 
	telnet 127.0.0.1 9191

h: fs.img
	$(QEMU) -machine virt -cpu cortex-a72 -m 128 -kernel $< -gdb tcp::11451 -nographic -singlestep -S

g:
	gdb-multiarch -n -x .gdbinit
	# aarch64-linux-gdb -x .gdbinit

c:
	$(RM) fs.img $K/kernel.elf $(OBJS)

rr: 
	make c
	make a
	make r