#include "types.h"
#include "defs.h"
#include "proc.h"
#include "arm.h"

void svcintr(struct trapframe* f, uint32 el, uint32 esr)
{
    struct proc* p = myproc();

    if (p != NULL && el == 0)
    {
        p->trapframe = f;
    }

    syscall();

    if (p != NULL && el == 0 && killed(p))
        exit(-1);
}

void defintr(struct trapframe* f, uint32 el, uint32 esr)
{
    //intr_off();
    //printf("default exception!\n");
}

void irqintr(struct trapframe* f, uint32 el, uint32 esr)
{
    struct proc* p = myproc();

    if (p != NULL && el == 0)   // from lower el
    {
        p->trapframe = f;
    }

    irqhandle(f, el);

    // ?
    if (p != NULL && el == 0 && killed(p))
        exit(-1);
}

void errintr(uint64 type)
{
    //intr_off();
    //panic("interrupt type %d not implemented!\n", type);
    printf("error exception: %d\n", type);
}