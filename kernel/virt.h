#ifndef __KERNEL_VIRT_H
#define __KERNEL_VIRT_H
#include "mmu.h"

#define NCPU            4  
#define PSCI_CPU_ON     0xc4000003

/* ARCH_TIMER_NS_EL1_IRQ */
#define IRQ_TIMER0      14
#define IRQ_UART        1
#define IRQ_MMIO        16

/* Private Peripheral Interrupts */
#define PPI_BASE        16
#define PPI2ID(x)       x + PPI_BASE

/* Shared Peripheral Interrupt */
#define SPI_BASE        32
#define SPI2ID(x)       x + SPI_BASE

#endif