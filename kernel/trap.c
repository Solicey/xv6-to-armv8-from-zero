#include "types.h"
#include "defs.h"
#include "proc.h"
#include "arm.h"

void svcintr(struct trapframe* f, uint32 el, uint32 esr)
{
    cprintf("svc exception!\n");
}

void defaultintr(struct trapframe* f, uint32 el, uint32 esr)
{
    // TODO: cli()
    cprintf("default exception!\n");
}

void irqintr(struct trapframe* f, uint32 el, uint32 esr)
{
    irqhandle(f);
}

void errorintr(uint64 type)
{
    //panic("interrupt type %d not implemented!\n", type);
    cprintf("error exception: %d\n", type);
}