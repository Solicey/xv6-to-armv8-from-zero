.section .text
.global _entry
_entry:
        ldr     x30, =stack_top
        mov     sp, x30
        bl      start
        b       .                       // dead loop
