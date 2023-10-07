QEMU = qemu-system-aarch64

K = kernel

OBJS = \
	$K/entry.o \
	$K/start.o \
	$K/uart.o

TOOLPREFIX = aarch64-linux-gnu-

CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
AS = $(TOOLPREFIX)as
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

RM = rm -f

CFLAGS = -Wall -g -O2 -fno-pie -fno-pic -mcmodel=large -march=armv8-a -mtune=cortex-a57 -fno-stack-protector -static -fno-builtin -nostdlib -ffreestanding -nostartfiles -mgeneral-regs-only -MMD -MP -Iinc
LDFLAGS = -L. -z max-page-size=4096
ASFLAGS = -march=armv8-a

.PHONY: all

all: fs.img

$K/kernel.elf: $(OBJS) Makefile $K/kernel.ld
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $@ $(OBJS)
	$(OBJDUMP) -S -D $@ > $K/kernel.asm
	$(OBJDUMP) -x $@ > $K/kernel.hdr

fs.img: $K/kernel.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.S
	$(AS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PRECIOUS: %.o

run: fs.img
	$(QEMU) -machine virt -cpu cortex-a57 -m 128 -kernel $< -nographic

qemu-gdb: fs.img
	$(QEMU) -machine virt -cpu cortex-a57 -m 128 -kernel $< -gdb tcp::11451 -nographic -singlestep -S

gdb:
	gdb-multiarch -n -x .gdbinit
	# aarch64-linux-gdb -x .gdbinit

clean:
	$(RM) fs.img $K/kernel.elf $(OBJS)
