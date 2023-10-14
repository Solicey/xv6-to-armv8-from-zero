#include "arm.h"
#include "defs.h"
#include "mmu.h"

extern char edata[], ebss[], vectors[];
extern uint64 kpgdir[];

void start()
{
    // clear bss
    memset((void*)edata, 0, ebss - edata);

    consoleinit();
    kinit();

    lvbar(vectors);
    gicinit();
    uartintr();
    timerinit();
    sti();

    //asm volatile("swi #0");

    //mappages(kpgdir, 0x0000000047000000, 0x46000000, 0x20000, AP_KERNEL);

    for (;;);
}