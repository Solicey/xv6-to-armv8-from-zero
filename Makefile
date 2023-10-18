QEMU = qemu-system-aarch64

K = kernel
U = user

OBJS = \
	$K/entry.o \
	$K/start.o \
	$K/uart.o \
	$K/kstruct.o \
	$K/console.o \
	$K/string.o \
	$K/kalloc.o \
	$K/vm.o \
	$K/trapasm.o \
	$K/trap.o \
	$K/gic.o \
	$K/timer.o \
	$K/spinlock.o \
	$K/proc.o \
	$K/swtch.o \
	$K/syscall.o \
	$K/sysproc.o \
	$K/sysfile.o

TOOLPREFIX = aarch64-linux-gnu-

CC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

RM = rm -f

CFLAGS = -Wall -Werror -g -fno-pie -fno-pic \
		 -mcmodel=large -march=armv8-a -mcpu=cortex-a72 \
		 -fno-stack-protector -static -fno-builtin \
		 -nostdlib -ffreestanding -nostartfiles \
		 -mgeneral-regs-only -MMD -MP -Ikernel -Iuser \
		 -Wno-incompatible-pointer-types \
		 -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS = -Lkernel -z max-page-size=4096
ASFLAGS = -march=armv8-a

.PHONY: all

a: fs.img

$K/kernel.elf: $(OBJS) Makefile $K/kernel.ld $U/initcode
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $@ $(OBJS) $U/initcode
	$(OBJDUMP) -S $@ > $K/kernel.asm
	$(OBJDUMP) -x $@ > $K/kernel.hdr

fs.img: $K/kernel.elf
	$(OBJCOPY) -O binary $< $@

$K/%.o: $K/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$U/initcode: $U/initcode.S
	$(CC) -Ikernel -nostdlib -c -o $U/initcode.o $U/initcode.S
	$(LD) -Lkernel -N -e start -Ttext 0 -r -o $U/initcode.out $U/initcode.o

	size -x -t $U/initcode.out | awk 'NR==2{print $$1}' | \
	xargs -I{} $(OBJCOPY) --add-symbol _binary_initcode_size={} \
	--prefix-symbols="_binary_initcode_" $U/initcode.out $@

	$(OBJDUMP) -S $@ > $U/initcode.asm
	$(OBJDUMP) -x $@ > $U/initcode.hdr

# .PRECIOUS: %.o

NCPU = 3
QEMURUN = $(QEMU) -machine virt  \
		-cpu cortex-a72 -m 128 \
		-smp $(NCPU) -kernel $< -nographic 

r: fs.img
	$(QEMURUN)

hh: fs.img
	$(QEMURUN) -monitor telnet:127.0.0.1:9191,server,nowait

gg: 
	telnet 127.0.0.1 9191

h: fs.img
	$(QEMURUN) -gdb tcp::11451 -singlestep -S

g:
	gdb-multiarch -n -x .gdbinit
	# aarch64-linux-gdb -x .gdbinit

c:
	$(RM) fs.img $K/kernel.elf $(OBJS) $U/initcode.o $U/initcode.out $U/initcode

rr: 
	make c
	make a
	make r

dtb:
	$(QEMU) -machine virt,dumpdtb=dump.dtb -cpu cortex-a72 -m 128 -smp $(NCPU)
	dtc -o dump.dts -O dts -I dtb dump.dtb
