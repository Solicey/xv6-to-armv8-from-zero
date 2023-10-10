// Physical memory layout
// hw/arm/virt.c
#ifndef __KERNEL_MEMLAYOUT_H
#define __KERNEL_MEMLAYOUT_H

#define DEV_MEM_SIZE        0x01000000

#define GIC                 0x08000000l

// qemu puts UART registers here in physical memory.
#define UART0               0x09000000l

#define MMIO                0x0a000000l

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x40000000 to PHYSTOP.
#define PHY_BASE            0x40000000l
#define PHY_STOP            (PHY_BASE + 0x08000000)

#define KERN_BASE           0xffffffff00000000l
// we first map 2MB low memory containing kernel code.
#define INIT_KERN_SIZE      0x200000

#endif