#include "types.h"
#include "memlayout.h"
#include "defs.h"

// start() jumps here
void main()
{
    kinit();            // physical page allocator
    kvminit();          // create kernel page table
    kvminithart();      // turn on paging

    uartinit();
    _putstr("uartinit done!\n");

    while (1) {}
}