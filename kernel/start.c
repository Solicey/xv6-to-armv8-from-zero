#include "defs.h"
#include "mmu.h"
#include "arm.h"
#include "virt.h"

extern char edata[], ebss[], vectors[];
extern uint64 kpgdir[];

volatile static int started = 0;
void start()
{
    if (cpuid() == 0)
    {
        // clear bss
        memset((void*)edata, 0, ebss - edata);

        consoleinit();

        cprintf("\n");
        cprintf("xv6 kernel is booting\n");
        cprintf("\n");

        kinit();

        lvbar(vectors);
        gicinit();
        uartintr();
        timerinit();
        intr_on();

        procinit();
        userinit();

        __sync_synchronize();
        started = 1;

        scheduler();
        for (;;);
    }
    else
    {
        while (started == 0);
        __sync_synchronize();
        cprintf("hart %d starting\n", cpuid());
        //assert(0);
        lvbar(vectors);
        gicinit();
        timerinit();
        intr_on();

        for (;;);
    }

    //asm volatile("swi #0");

    //mappages(kpgdir, 0x0000000047000000, 0x46000000, 0x20000, AP_KERNEL);


}