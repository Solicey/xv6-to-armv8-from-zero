#include "defs.h"

void start()
{
    consoleinit();
    cprintf("test: 0x%x, %ld, %lld, %p, %c, %s\n", 0x114514, 0xffffffff, 0x0fffffffffffffffl, \
        0x40000000, 't', "test");
    panic("In %d they voted my city the worst place to live in America\n", 2077);

    for (;;);
}