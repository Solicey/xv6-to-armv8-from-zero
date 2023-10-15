#include "memlayout.h"
#include "mmu.h"
#include "sysregs.h"
#include "virt.h"

.section .text
.global _entry

_entry:
    /* Read cpu id */
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

halt:
    wfe
    b       halt
