#ifndef __KERNEL_ARM_H
#define __KERNEL_ARM_H

#include "types.h"

/* ARCH_TIMER_NS_EL1_IRQ */
#define IRQ_TIMER0      14
#define IRQ_UART0       1

/* Private Peripheral Interrupts */
#define PPI_BASE        16
#define PPI2ID(x)       x + PPI_BASE

/* Shared Peripheral Interrupt */
#define SPI_BASE        32
#define SPI2ID(x)       x + SPI_BASE

/* Unmask DAIF to start interrupt. */
static inline void sti()
{
    asm volatile("msr daifclr, #2");
}

/* Mask DAIF to close interrupt. */
static inline void cli()
{
    asm volatile("msr daifset, #2");
}

/* Brute-force data and instruction synchronization barrier. */
static inline void disb()
{
    asm volatile("dsb sy; isb");
}

/* Load vector base (virtual) address register (EL1). */
static inline void lvbar(void *vaddr)
{
    disb();
    asm volatile("msr vbar_el1, %[x]" : : [x] "r"(vaddr) : );
    disb();
    cprintf("load intr vector done!\n");
}


#endif
