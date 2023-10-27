#include "types.h"
#include "defs.h"
#include "proc.h"

uint64 sys_exit(void)
{
    int n;
    argint(1, &n);
    exit(n);
    return 0;
}

uint64 sys_fork(void)
{
    return fork();
}

uint64 sys_wait(void)
{
    uint64 p;
    argaddr(1, &p);
    return wait(p);
}

uint64 sys_yield(void)
{
    yield();
    return 0;
}

uint64 sys_sbrk(void)
{
    uint64 addr;
    int n;

    argint(1, &n);
    addr = myproc()->size;
    if (growproc(n) < 0)
        return -1;
    return addr;
}