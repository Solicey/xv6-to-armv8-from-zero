#include "types.h"
#include "defs.h"
#include "proc.h"
#include "arm.h"

void svcintr(struct trapframe* f, uint32 el, uint32 esr)
{
    struct proc* p = myproc();

    p->trapframe = f;
    syscall();
}

void defintr(struct trapframe* f, uint32 el, uint32 esr)
{
    //intr_off();
    //cprintf("default exception!\n");
}

void irqintr(struct trapframe* f, uint32 el, uint32 esr)
{
    struct proc* p = myproc();

    if (p != NULL)
    {
        p->trapframe = f;
    }

    irqhandle(f);
}

void errintr(uint64 type)
{
    //intr_off();
    //panic("interrupt type %d not implemented!\n", type);
    //cprintf("error exception: %d\n", type);
}