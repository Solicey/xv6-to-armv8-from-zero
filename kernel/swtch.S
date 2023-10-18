# Context switch
#
#   void swtch(struct context *old, struct context *new);
# 
# Save current registers in old. Load from new.	

.global swtch
swtch:
    add     x9, x0, #0x80
    mov     x10, sp
    stp     x30, x10, [x9, #-16]!
    stp     x28, x29, [x9, #-16]!
    stp     x26, x27, [x9, #-16]!
    stp     x24, x25, [x9, #-16]!
    stp     x22, x23, [x9, #-16]!
    stp     x20, x21, [x9, #-16]!
    stp     x18, x19, [x9, #-16]!
    stp     x16, x17, [x9, #-16]!

    mov     x9, x1
    ldp     x16, x17, [x9], #16
    ldp     x18, x19, [x9], #16
    ldp     x20, x21, [x9], #16
    ldp     x22, x23, [x9], #16
    ldp     x24, x25, [x9], #16
    ldp     x26, x27, [x9], #16
    ldp     x28, x29, [x9], #16
    ldp     x30, x10, [x9], #16
    mov     sp, x10

    br      x30
