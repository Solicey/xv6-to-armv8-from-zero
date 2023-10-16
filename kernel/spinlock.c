#include "spinlock.h"
#include "defs.h"
#include "types.h"

// Check whether this cpu is holding the lock.
/*int holding(struct spinlock* lk)
{
    int r;
    r = (lk->locked && lk->cpu == mycpu());
    return r;
}*/

void acquire(struct spinlock* lk)
{
    int tmp;
    asm volatile(
        "1: ldxr    %0, %1\n"       // 
        "   cbnz    %0, 1b\n"
        "   stxr    %w0, %2, %1\n"
        "   cbnz    %0, 1b\n"
        : "=&r"(tmp), "+Q"(lk->locked)
        : "r"(1)
        : "memory"
        );
    __sync_synchronize();
    //while (lk->locked || __sync_lock_test_and_set(&lk->locked, __ATOMIC_ACQUIRE));
}

void release(struct spinlock* lk)
{
    assert(lk->locked);

    __sync_synchronize();
    lk->locked = 0;
    //__sync_lock_release(&lk->locked, __ATOMIC_RELEASE);
}