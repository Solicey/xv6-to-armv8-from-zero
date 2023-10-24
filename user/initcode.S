# Initial process that execs /init.
# This code runs in user space.

#include "kernel/syscall.h"

.global start

/* exec(init, argv) */
start:
    ldr     x1, =init
    ldr     x2, =argv
    mov     x0, #SYS_exec
    svc     0x00

exit:
    /* mov     x0, #SYS_exit */
    /* svc     0x00 */
    b       exit

init:
    .string "/init\0"

.p2align 4
argv:
    .word   init
    .word   0
    .word   0
    .word   0

