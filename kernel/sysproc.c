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