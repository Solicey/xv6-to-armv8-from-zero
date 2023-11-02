#include "types.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"

struct spinlock tickslock;
uint ticks;

#define TICK_PER_SEC    50

void timerset(void)
{
    unsigned long value, freq, cnt, cmp;
    value = 0;
    asm volatile("msr cntp_ctl_el0, %0" : : "r"(value) : );
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq) : : );
    asm volatile("mrs %0, cntpct_el0" : "=r"(cnt) : : );
    cmp = cnt + (freq / 1000) * TICK_PER_SEC;
    asm volatile("msr cntp_cval_el0, %0" : : "r"(cmp));
    value = 1;
    asm volatile("msr cntp_ctl_el0, %0" : : "r"(value));
}

void timerinit(void)
{
    intrset(PPI2ID(IRQ_TIMER0), timerintr);
    initlock(&tickslock, "time");
    ticks = 0;
}

void timerintr(struct trapframe* f, int id, uint32 el)
{
    timerset();
    if (cpuid() == 0)
    {
        acquire(&tickslock);
        ticks++;
        wakeup(&ticks);
        release(&tickslock);
    }
}