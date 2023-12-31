#include "kernel/syscall.h"

.macro sys_call func, funcid
.global \func
\func:
    str     x5, [sp, #-0x08]!
    mov     x5, x4
    mov     x4, x3
    mov     x3, x2
    mov     x2, x1
    mov     x1, x0
    mov     x0, #\funcid
    svc     0x0
    ldr     x5, [sp], #0x08
    br      x30
.endm

#define SYSCALL(func)           \
.global func;                   \
func:                           \
    str     x5, [sp, #-0x08]!;  \
    mov     x5, x4;             \
    mov     x4, x3;             \
    mov     x3, x2;             \
    mov     x2, x1;             \
    mov     x1, x0;             \
    mov     x0, #SYS_##func;    \
    svc     0x0;                \
    ldr     x5, [sp], #0x08;    \
    br      x30;                

SYSCALL(exec)
SYSCALL(exit)
SYSCALL(write)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(dup)
SYSCALL(fork)
SYSCALL(wait)
SYSCALL(yield)
SYSCALL(read)
SYSCALL(chdir)
SYSCALL(close)
SYSCALL(sbrk)
SYSCALL(fstat)
SYSCALL(mkdir)
SYSCALL(unlink)
SYSCALL(sleep)
SYSCALL(pipe)
SYSCALL(getpid)
SYSCALL(procdump)
SYSCALL(kill)
SYSCALL(link)
SYSCALL(uptime)