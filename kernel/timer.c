#include "types.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"
// timer frequency
static uint64 timerfq;

void timerinit(void)
{
    asm volatile("mrs %[r], cntfrq_el0" : [r] "=r"(timerfq) : : );
    printf("timer frq: %d\n", timerfq);    // 62500000
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(timerfq) : );
    intrset(PPI2ID(IRQ_TIMER0), timerintr);
    asm volatile("msr cntp_ctl_el0, %[x]" : : [x] "r"(1) : );

    printf("timerinit done!\n");
}

void timerintr(struct trapframe* f, int id, uint32 el)
{
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(timerfq) : );
    // give up cpu on lower el
    if (el == 0)
    {
        //printf("el0 timer! hart %d\n", cpuid());
        struct proc* p = myproc();
        if (p && p->state == RUNNING)
            yield();
    }
}