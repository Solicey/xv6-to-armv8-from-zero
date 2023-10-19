#include "types.h"
#include "defs.h"
#include "proc.h"
#include "param.h"

uint64 sys_exec(void)
{
    char path[MAXPATH], *argv[MAXARG];
    int i;
    uint64 uargv, uarg;

    argaddr(2, &uargv);
    cprintf("exec uargv: 0x%p\n", uargv);
    if (argstr(1, path, MAXPATH) < 0)
        return -1;
    cprintf("exec path: %s\n", path);

    memset(argv, 0, sizeof(argv));
    for (i = 0; ; i++)
    {
        if (i >= NELEM(argv))
            goto bad;
        if (fetchaddr(uargv + sizeof(uint64) * i, (uint64*)&uarg) < 0)
            goto bad;
        if (uarg == 0)
        {
            argv[i] = 0;
            break;
        }
        argv[i] = kalloc();
        if (argv[i] == NULL)
            goto bad;
        if (fetchstr(uarg, argv[i], PG_SIZE) < 0)
            goto bad;
        cprintf("   exec argv[%d]: %s\n", i, argv[i]);
    }

    // TODO: exec
    int ret = 0;
    cprintf("exec get ret!\n");

    for (i = 0; i < NELEM(argv) && argv[i] != 0; i++)
        kfree(argv[i]);

    return ret;

bad:
    for (i = 0; i < NELEM(argv) && argv[i] != 0; i++)
        kfree(argv[i]);
    return -1;
}