#include "types.h"
#include "defs.h"

void timerinit()
{
    uint64 frq;
    asm volatile("mrs %[r], cntfrq_el0" : [r] "=r"(frq) : : );
    cprintf("timer frq: %d\n", frq);    // 62500000
    asm volatile("msr cntp_tval_el0, %[x]" : : [x] "r"(frq) : );
    asm volatile("msr cntp_ctl_el0, %[x]" : : [x] "r"(1) : );

}