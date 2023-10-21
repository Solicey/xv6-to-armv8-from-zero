// Physical memory layout
// hw/arm/virt.c
#ifndef __KERNEL_MEM_LAYOUT_H
#define __KERNEL_MEM_LAYOUT_H

#include "mmu.h"
#include "virt.h"

#define GIC_BASE        0x08000000l    

// qemu puts UART registers here in physical memory.
#define UART_BASE       0x09000000l

#define MMIO_BASE       0x0a000000l

#define PHY_BASE        0x40000000l
#define PHY_LINK        (PHY_BASE + 0x0l)
#define PHY_STOP        (PHY_BASE + 0x8000000l)     // 128M memory

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x40000000 to PHYSTOP.
#define KERN_BASE       0xffff000000000000l
#define KERN_LINK       (KERN_BASE + PHY_LINK)

#define V2P(x)          ((uint64)(x) - KERN_BASE)
#define P2V(x)          ((uint64)(x) + KERN_BASE)    

// size of initial kernel stack
#define KSTACK_SIZE     PG_SIZE * NCPU

#endif