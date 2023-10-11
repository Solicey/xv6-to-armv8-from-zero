#include "memlayout.h"
#include "mmu.h"
#include "sysregs.h"

.section .text
.global _entry

_entry:
    /* Read cpu id, stop slave cores. */
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbnz    x1, halt

    /* Get current exception level by CurrentEL[3:2]. */
    mrs     x1, CurrentEL
    and     x1, x1, #0xc
    cmp     x1, #8
    bl      el1
    bge     halt

el1:
    adr     x0, kpgdir

    /* Higher and lower half map to same address. */
    msr     ttbr0_el1, x0
    msr     ttbr1_el1, x0

    ldr     x0, =(TCR_VALUE)        
    msr     tcr_el1, x0

    ldr     x0, =(MAIR_VALUE)
    msr     mair_el1, x0

    /* Modify System Control Register to enable MMU. */
    mrs     x0, sctlr_el1
    orr     x0, x0, #SCTLR_MMU_ENABLED 
    msr     sctlr_el1, x0

    /* Set up stack pointer. */
    ldr     x0, =stack_top
	ldr     x1, =(KERN_BASE)
	add     sp, x1, x0

    /*
     * Jump to C code, should not return.
     * Why not directly run `b start`?
     */ 
    ldr     x1, =start
    br      x1

halt:
    wfe
    b       halt
