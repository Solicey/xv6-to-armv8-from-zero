#include "memlayout.h"
#include "mmu.h"
#include "sysregs.h"
#include "virt.h"

.section .text

_entry:
    /* Get current exception level by CurrentEL[3:2]. */
    mrs     x1, CurrentEL
    and     x1, x1, #0xc
    cmp     x1, #8
    beq     el2
    bgt     el3
    b       el1

el3:
    mov     x9, #SCR_VALUE
    msr     scr_el3, x9
    mov     x9, #SPSR_EL3_VALUE
    msr     spsr_el3, x9
    adr     x9, el2
    msr     elr_el3, x9
    eret

el2:
    /*
     * HCR_EL2.RW, bit[31] = 1: The Execution state for EL1 is AArch64.
     * The Execution state for EL0 is determined by the current
     * value of PSTATE.nRW when executing at EL0.
     */
    mov     x9, #HCR_VALUE
    msr     hcr_el2, x9

    /* Setup SCTLR access. */
    ldr     x9, =SCTLR_VALUE_MMU_DISABLED
    msr     sctlr_el1, x9

    /* Change execution level to EL1. */
    mov     x9, #SPSR_EL2_VALUE
    msr     spsr_el2, x9
    adr     x9, el1
    msr     elr_el2, x9
    eret

el1:
    adr     x0, kpgdir

    /* Higher and lower half map to same address. */
    msr     ttbr0_el1, x0
    msr     ttbr1_el1, x0

    ldr     x0, =(TCR_VALUE)        
    msr     tcr_el1, x0

    ldr     x0, =(MAIR_VALUE)
    msr     mair_el1, x0

    isb

    /* Modify System Control Register to enable MMU. */
    mrs     x0, sctlr_el1
    orr     x0, x0, #SCTLR_MMU_ENABLED 
    msr     sctlr_el1, x0

    isb

    /* Read cpu id */
    mrs     x0, mpidr_el1
    and     x0, x0, #3

    /* Set up stack pointer. Using sp_el1 */
    msr     spsel, #1
    ldr     x2, =kstack
    add     x2, x2, #KSTACK_SIZE
    mov     x1, #PG_SIZE
    mul     x1, x1, x0
    sub     x2, x2, x1
	ldr     x1, =(KERN_BASE)
	add     sp, x1, x2

    cbnz    x0, to_start

cpu_enb:
    /* Enable cpu 1 & 2 */
    ldr     x0, =(PSCI_CPU_ON)
    mov     x1, #1
    ldr     x2, =(PHY_LINK)
    mov     x3, #0
    hvc     #0
    
    ldr     x0, =(PSCI_CPU_ON)
    mov     x1, #2
    ldr     x2, =(PHY_LINK)
    mov     x3, #0
    hvc     #0

to_start:
    /*
     * Jump to C code, should not return.
     * Why not directly run `b start`?
     */ 
    ldr     x1, =start
    br      x1

spin:
    wfe
    b       spin
