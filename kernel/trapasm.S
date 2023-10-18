.macro ventry_el0 label
    .align  7
	el0_store_trap_frame
	b 		\label
.endm

.macro ventry_el1 label
	.align  7
	el1_store_trap_frame
	b 		\label
.endm

.macro verror type
    .align  7
    mov     x0, \type
    b       errintr
.endm

.macro set_param level
	mov 	x0, sp
	mov		x1, \level
	mrs 	x2, esr_el1
.endm

.macro el0_store_trap_frame
    /*
     * Build your trap frame.
     * Hint:
     * 1. Use stp/ldp in favor of str/ldr to maintain stack alignment
          For example, `stp x1, x2, [sp, #-16]!` is equivalent to 
          first `push x2` and then `push x1`. Be careful about the order.
     * 2. Use mrs/msr to move from/to system registers such as
     *    ELR_EL1, SPSR_EL1, and SP_EL0.
     */
    sub     sp, sp, #0x20           /* reserverd for LR, SP, SPSR and ELR */
    stp	    x28, x29, [sp, #-16]!	/* pre-index */
	stp	    x26, x27, [sp, #-16]!
	stp	    x24, x25, [sp, #-16]!
	stp	    x22, x23, [sp, #-16]!
	stp	    x20, x21, [sp, #-16]!
	stp	    x18, x19, [sp, #-16]!
	stp	    x16, x17, [sp, #-16]!
	stp	    x14, x15, [sp, #-16]!
	stp	    x12, x13, [sp, #-16]!
	stp	    x10, x11, [sp, #-16]!
	stp	    x8, x9, [sp, #-16]!
	stp	    x6, x7, [sp, #-16]!
	stp	    x4, x5, [sp, #-16]!
	stp	    x2, x3, [sp, #-16]!
	stp	    x0, x1, [sp, #-16]!
	mrs     x9, sp_el0
	mrs	    x10, elr_el1                /* ELR */
	mrs	    x11, spsr_el1               /* SPSR */
	stp	    x30, x9, [sp, #0xf0]        /* LR, SP */
	stp	    x10, x11, [sp, #0x100]      /* ELR, SPSR */
.endm

.macro el1_store_trap_frame
    sub     sp, sp, #0x20           /* reserverd for LR, SP, SPSR and ELR */
    stp	    x28, x29, [sp, #-16]!	/* pre-index */
	stp	    x26, x27, [sp, #-16]!
	stp	    x24, x25, [sp, #-16]!
	stp	    x22, x23, [sp, #-16]!
	stp	    x20, x21, [sp, #-16]!
	stp	    x18, x19, [sp, #-16]!
	stp	    x16, x17, [sp, #-16]!
	stp	    x14, x15, [sp, #-16]!
	stp	    x12, x13, [sp, #-16]!
	stp	    x10, x11, [sp, #-16]!
	stp	    x8, x9, [sp, #-16]!
	stp	    x6, x7, [sp, #-16]!
	stp	    x4, x5, [sp, #-16]!
	stp	    x2, x3, [sp, #-16]!
	stp	    x0, x1, [sp, #-16]!
	add     x9, sp, #0x110
	mrs	    x10, elr_el1                /* ELR */
	mrs	    x11, spsr_el1               /* SPSR */
	stp	    x30, x9, [sp, #0xf0]        /* LR, SP */
	stp	    x10, x11, [sp, #0x100]      /* ELR, SPSR */
.endm



.global vectors

.align  12
vectors:
el1_sp0:
    verror(0)
    verror(1)
    verror(2)
    verror(3)

el1_spx:
    verror(4)
    ventry_el1(current_el_irq)					/* for debug only */
    verror(6)
    verror(7)

/* Lower EL using AArch64 */
el0_aarch64:
    ventry_el0(lower_el_sync)					/* Synchronous */
    ventry_el0(lower_el_irq)           			/* IRQ/vIRQ */
    verror(10)
    verror(11)

el0_aarch32:
    verror(12)
    verror(13)
    verror(14)
    verror(15)



el0_load_trap_frame_and_ret:
	ldp		x10, x11, [sp, #0x100]   	/* ELR, SPSR */
	ldp		x30, x9, [sp, #0xf0]    	/* LR, SP */

	msr		elr_el1, x10             	/* ELR */
	msr		spsr_el1, x11
	msr 	sp_el0, x9

	mov 	x29, sp
	add 	sp, sp, #0x110

	ldp		x0, x1, [x29], #16			/* post-index */
	ldp		x2, x3, [x29], #16
	ldp		x4, x5, [x29], #16
	ldp		x6, x7, [x29], #16
	ldp		x8, x9, [x29], #16
	ldp		x10, x11, [x29], #16
	ldp		x12, x13, [x29], #16
	ldp		x14, x15, [x29], #16
	ldp		x16, x17, [x29], #16
	ldp		x18, x19, [x29], #16
	ldp		x20, x21, [x29], #16
	ldp		x22, x23, [x29], #16
	ldp		x24, x25, [x29], #16
	ldp		x26, x27, [x29], #16
	ldr 	x28, [x29], #8
	ldr 	x29, [x29]
	eret

el1_load_trap_frame_and_ret:
	ldp		x10, x11, [sp, #0x100]   	/* ELR, SPSR */
	ldp		x30, x9, [sp, #0xf0]    	/* LR, SP */

	msr		elr_el1, x10             	/* ELR */
	msr		spsr_el1, x11

	mov 	x29, sp
	mov 	sp, x9

	ldp		x0, x1, [x29], #16			/* post-index */
	ldp		x2, x3, [x29], #16
	ldp		x4, x5, [x29], #16
	ldp		x6, x7, [x29], #16
	ldp		x8, x9, [x29], #16
	ldp		x10, x11, [x29], #16
	ldp		x12, x13, [x29], #16
	ldp		x14, x15, [x29], #16
	ldp		x16, x17, [x29], #16
	ldp		x18, x19, [x29], #16
	ldp		x20, x21, [x29], #16
	ldp		x22, x23, [x29], #16
	ldp		x24, x25, [x29], #16
	ldp		x26, x27, [x29], #16
	ldr 	x28, [x29], #8
	ldr 	x29, [x29]
	eret


current_el_irq:
	set_param(1)
	bl 		irqintr
	b 		el1_load_trap_frame_and_ret

/*
* Bits [31:26] of ESR_ELn indicate the exception class 
* which allows the handler to distinguish between the various possible 
* exception causes
*/
lower_el_sync:
	mrs		x2, esr_el1
	lsr 	x24, x2, #26
	cmp 	x24, #0x15			/* 0b010101: SVC instruction execution in AArch64 state. */
	beq 	el0_svc
	b 		el0_default

el0_svc:
	set_param(0)
	bl 		svcintr
	b 		el0_load_trap_frame_and_ret

el0_default:
	set_param(0)
	bl  	defintr
	b 		.

lower_el_irq:
	set_param(0)
	bl 		irqintr
	b 		el0_load_trap_frame_and_ret


.global trapret
trapret:
	b 		el0_load_trap_frame_and_ret