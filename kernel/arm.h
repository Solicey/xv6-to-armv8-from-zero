#ifndef __KERNEL_ARM_H
#define __KERNEL_ARM_H

#include "defs.h"
#include "types.h"
#include "sysregs.h"

/* Unmask DAIF to start interrupt. */
static inline void intr_on()
{
    asm volatile("msr daifclr, #2");
}

/* Mask DAIF to close interrupt. */
static inline void intr_off()
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
}

static inline int cpuid()
{
    int64 id;
    asm volatile("mrs %[x], mpidr_el1" : [x] "=r"(id) : : );
    return id & 0xff;
}

static inline int intr_get()
{
    int64 x;
    asm volatile("mrs %[x], daif" : [x] "=r"(x) : : );
    return !(x & DAIF_I);
}

/* Load Translation Table Base Register 0 (EL1). */
static inline void lttbr0(uint64 paddr)
{
    asm volatile("msr ttbr0_el1, %[x]" : : [x] "r"(paddr) : );
    disb();
    asm volatile("tlbi vmalle1");
    disb();
}

/* Load Translation Table Base Register 1 (EL1). */
static inline void lttbr1(uint64 paddr)
{
    asm volatile("msr ttbr1_el1, %[x]" : : [x] "r"(paddr) : );
    disb();
    asm volatile("tlbi vmalle1");
    disb();
}

#endif