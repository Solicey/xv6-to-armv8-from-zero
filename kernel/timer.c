#include "types.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"
// timer frequency
static uint64 timerfq;
static int ticks_per_sec = 20;
static int ticks_interval;

struct spinlock tickslock;
uint ticks;

void timerinit(void)
{
    asm volatile("mrs %[r], cntfrq_el0" : [r] "=r"(timerfq) : : );
    //printf("timer frq: %d\n", timerfq);    // 62500000
    ticks_interval = timerfq / ticks_per_sec;
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(ticks_interval) : );
    intrset(PPI2ID(IRQ_TIMER0), timerintr);
    asm volatile("msr cntp_ctl_el0, %[x]" : : [x] "r"(1) : );

    initlock(&tickslock, "time");
    ticks = 0;
    //printf("timerinit done!\n");
}

void timerintr(struct trapframe* f, int id, uint32 el)
{
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(ticks_interval) : );
    //printf("timer! hart %d\n", cpuid());
    if (cpuid() == 0)
    {
        acquire(&tickslock);
        ticks++;
        wakeup(&ticks);
        release(&tickslock);
    }
    // give up cpu on lower el
    if (el == 0)
    {
        //printf("el0 timer! hart %d pid %d\n", cpuid(), myproc()->pid);
        // ! no return
        yield();
    }
}