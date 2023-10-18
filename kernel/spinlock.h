#ifndef __KERNEL_SPINLOCK_H
#define __KERNEL_SPINLOCK_H

#include "types.h"

struct cpu;

struct spinlock
{
    volatile uint locked;

    // for debugging
    char* name;             // name of lock
    struct cpu* cpu;        // the cpu holding the lock
};


#endif