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
	$K/sysfile.o \
	$K/bio.o \
	$K/sleeplock.o \
	$K/virtio_disk.o \
	$K/fs.o \
	$K/log.o \
	$K/file.o \
	$K/exec.o \
	$K/printf.o

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
		 -mgeneral-regs-only -MMD -MP -I. \
		 -Wno-incompatible-pointer-types \
		 -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS = -Lkernel -z max-page-size=4096
ASFLAGS = -march=armv8-a

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: $K/%.o $U/%.o

.PHONY: all

all: fs.img $K/kernel

a: fs.img $K/kernel

$K/kernel: $(OBJS) Makefile $K/kernel.ld $U/initcode
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $@ $(OBJS) $U/initcode
	$(OBJDUMP) -S $@ > $K/kernel.asm
	$(OBJDUMP) -x $@ > $K/kernel.hdr

# fs.img: $K/kernel
# $(OBJCOPY) -O binary $< $@

$K/%.o: $K/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$U/initcode: $U/initcode.S
	$(CC) -I. -nostdlib -c -o $U/initcode.o $U/initcode.S
	$(LD) -L. -N -e start -Ttext 0 -r -o $U/initcode.out $U/initcode.o

	size -x -t $U/initcode.out | awk 'NR==2{print $$1}' | \
	xargs -I{} $(OBJCOPY) --add-symbol _binary_initcode_size={} \
	--prefix-symbols="_binary_initcode_" $U/initcode.out $@

	$(OBJDUMP) -S $@ > $U/initcode.asm
	$(OBJDUMP) -x $@ > $U/initcode.hdr

UPROGS = \
	$U/_init \
	$U/_sh

ULIB = $U/ulib.o $U/usys.o $U/printf.o

$U/%.o: $U/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$U/usys.o: $U/usys.S
	$(CC) $(CFLAGS) -c -o $@ $<

$U/_%: $U/%.o $(ULIB)
	$(LD) $(LDFLAGS) -T $U/user.ld -o $@ $^
	$(OBJDUMP) -S $@ > $U/$*.asm
	$(OBJDUMP) -x $@ > $U/$*.hdr

mkfs/mkfs: mkfs/mkfs.c $K/fs.h $K/param.h
	gcc -Werror -Wall -I. -o $@ $<

fs.img: mkfs/mkfs README $(UPROGS)
	mkfs/mkfs $@ README $(UPROGS)

NCPU = 3
QEMURUN = $(QEMU) -machine virt  \
		-cpu cortex-a72 -m 128 \
		-smp $(NCPU) -kernel $K/kernel -nographic \
		$(QEMUDISK)
		
QEMUDISK = -drive file=fs.img,if=none,format=raw,id=x0 \
		-device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 \
		-global virtio-mmio.force-legacy=false

r: $K/kernel fs.img
	$(QEMURUN)

hh: $K/kernel fs.img
	$(QEMURUN) -monitor telnet:127.0.0.1:9191,server,nowait

gg: 
	telnet 127.0.0.1 9191

h: $K/kernel fs.img
	$(QEMURUN) -gdb tcp::11451 -singlestep -S

g:
	gdb-multiarch -n -x .gdbinit
	# aarch64-linux-gdb -x .gdbinit

c:
	$(RM) fs.img $K/kernel $(OBJS) \
	$U/initcode.o $U/initcode.out $U/initcode \
	mkfs/mkfs $(UPROGS) */*.o */*.d */*.asm */*.hdr

rr: 
	make c
	make a
	make r

dt:
	$(QEMU) -machine virt,dumpdtb=dump.dtb \
			-cpu cortex-a72 -m 128 -smp $(NCPU) \
			$(QEMUDISK)
	dtc -o dump.dts -O dts -I dtb dump.dtb
