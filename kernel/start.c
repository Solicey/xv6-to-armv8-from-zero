#include "defs.h"

extern char edata[], ebss[];

void start()
{
    cprintf("edata: 0x%p, ebss: 0x%p\n", edata, ebss);
    memset((void*)edata, 0, ebss - edata);
    consoleinit();
    //cprintf("test: 0x%x, %ld, %lld, %p, %c, %s\n", 0x114514, 0xffffffff, 0x0fffffffffffffffl, 0x40000000, 't', "test");
    //panic("In %d they voted my city the worst place to live in America\n", 2077);

    for (;;);
}