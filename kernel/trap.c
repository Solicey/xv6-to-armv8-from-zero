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
    struct proc* p = myproc();

    //printf("an exception occurred! esr: 0x%x\n", esr);

    if (p != NULL && el == 0)
    {
        exit(-1);
    }
    //intr_off();
}

void irqintr(struct trapframe* f, uint32 el, uint32 esr)
{
    struct proc* p = myproc();

    if (p != NULL && el == 0)   // from lower el
    {
        p->trapframe = f;
    }

    int is_timer = irqhandle(f, el);

    // ?
    if (p != NULL && killed(p))
    {
        //printf("before ret found killed pid: %d\n", p->pid);
        exit(-1);
    }

    // give up cpu on lower el
    if (p != NULL && is_timer)
    {
        //printf("el0 timer! hart %d pid %d\n", cpuid(), myproc()->pid);
        // ! no return

        yield();
    }
}

void errintr(uint64 type)
{
    //intr_off();
    //panic("interrupt type %d not implemented!\n", type);
    //printf("unhandled exception! type: %d\n", type);
}