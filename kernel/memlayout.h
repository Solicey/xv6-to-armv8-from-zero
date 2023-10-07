// Physical memory layout
// hw/arm/virt.c

// qemu puts UART registers here in physical memory.
#define UART        0x09000000L
#define UART_IRQ    1

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x40000000 to PHYSTOP.
#define KERNBASE    0x40000000L
#define PHYSTOP     (KERNBASE + 128 * 1024 * 1024)