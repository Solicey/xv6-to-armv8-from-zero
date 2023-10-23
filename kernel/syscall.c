#include "types.h"
#include "defs.h"
#include "syscall.h"
#include "proc.h"

extern uint64 sys_exit(void);
extern uint64 sys_exec(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64(*syscalls[])(void) = {
[SYS_exit] sys_exit,
[SYS_exec] sys_exec
};

void syscall(void)
{
    uint64 num, ret;
    struct proc* p = myproc();

    num = p->trapframe->x0;
    if ((num > 0) && (num <= NELEM(syscalls)) && syscalls[num])
    {
        ret = syscalls[num]();

        // in ARM, parameters to main (argc, argv) are passed in r0 and r1
        // do not set the return value if it is SYS_exec (the user program
        // anyway does not expect us to return anything).
        if (num != SYS_exec)
        {
            p->trapframe->x0 = ret;
        }
    }
    else
    {
        cprintf("%d %s: syscall %d not implemented!\n", p->pid, p->name, num);
        p->trapframe->x0 = -1;
    }
}

// Fetch the uint64 at addr from the current process.
int fetchaddr(uint64 addr, uint64* ip)
{
    struct proc* p = myproc();
    if (addr >= p->sz || addr + sizeof(uint64) > p->sz)
        return -1;
    if (copyin(p->pagetable, (char*)ip, addr, sizeof(*ip)) != 0)
        return -1;
    return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetchstr(uint64 addr, char* buf, int max)
{
    struct proc* p = myproc();
    if (copyinstr(p->pagetable, buf, addr, max) < 0)
        return -1;
    return strlen(buf);
}

static uint64 argraw(int n)
{
    struct proc* p = myproc();
    switch (n)
    {
    case 0:
        return p->trapframe->x0;
    case 1:
        return p->trapframe->x1;
    case 2:
        return p->trapframe->x2;
    case 3:
        return p->trapframe->x3;
    case 4:
        return p->trapframe->x4;
    case 5:
        return p->trapframe->x5;
    }
    assert(0);
    return -1;
}

// Fetch the nth 32-bit system call argument.
void argint(int n, int* ip)
{
    *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void argaddr(int n, uint64* ip)
{
    *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int argstr(int n, char* buf, int max)
{
    uint64 addr;
    argaddr(n, &addr);
    return fetchstr(addr, buf, max);
}