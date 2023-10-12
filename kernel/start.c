#include "defs.h"
#include "mmu.h"

extern char edata[], ebss[];
extern uint64 kpgdir[];

void start()
{
    // clear bss
    memset((void*)edata, 0, ebss - edata);

    consoleinit();
    kinit();

    //mappages(kpgdir, 0x0000000047000000, 0x46000000, 0x20000, AP_KERNEL);

    for (;;);
}