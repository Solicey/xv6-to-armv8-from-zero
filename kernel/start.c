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

        printfinit();
        consoleinit();

        printf("\nxv6 kernel is booting\n\n");

        kinit();

        lvbar(vectors);
        gicinit();
        uartsti();
        timerinit();

        binit();
        iinit();
        fileinit();
        virtio_disk_init();

        procinit();
        userinit();

        __sync_synchronize();
        started = 1;
    }
    else
    {
        while (started == 0);
        __sync_synchronize();
        printf("hart %d starting\n", cpuid());

        //panic("PANIC!");
        lvbar(vectors);
        timerinit();
    }

    scheduler();
}