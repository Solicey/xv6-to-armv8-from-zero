#include "memlayout.h"

.section .text
.align 16

.global _entry
_entry:
	// initialize stack pointers for svc modes
	mov     x0, #1     // select SP_EL1
	msr     spsel, x0
	isb

    // MSR SPSel, #imm 
    // A value of 0 or 1 in this register is used to select
    // between using EL0 stack pointer or the current exception 
    // level stack pointer
	// 1 core stack for now

	adrp    x0, stack0
	mov     sp, x0

	// clear the bss section
	ldr     x1, =data_end
	ldr     x2, =end
	mov     x3, #0x00

1:
	cmp     x1, x2
	bge     2f
	str     x3, [x1]
	add     x1, x1, #0x08
	blt     1b
2:
	bl      start
	b       .
