#include "types.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"
// timer frequency
static uint64 timerfq;

void timerinit()
{
    asm volatile("mrs %[r], cntfrq_el0" : [r] "=r"(timerfq) : : );
    cprintf("timer frq: %d\n", timerfq);    // 62500000
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(timerfq) : );
    irqhset(PPI2ID(IRQ_TIMER0), timerirqh);
    asm volatile("msr cntp_ctl_el0, %[x]" : : [x] "r"(1) : );

    cprintf("timerinit done!\n");

    int x = 114;
    cprintf("x: %d\n", x);
}

void timerirqh(struct trapframe* f, int id)
{
    cprintf("timer! hart %d\n", cpuid());
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(timerfq) : );
}