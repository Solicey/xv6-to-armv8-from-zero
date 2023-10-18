#include "spinlock.h"
#include "defs.h"
#include "types.h"

void initlock(struct spinlock* lk, char* name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

void acquire(struct spinlock* lk)
{
    push_off(); // disable interrupts to avoid deadlock.
    assert(!holding(lk));

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

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that the critical section's memory
    // references happen strictly after the lock is acquired.
    __sync_synchronize();
    //while (lk->locked || __sync_lock_test_and_set(&lk->locked, __ATOMIC_ACQUIRE));

    // Record info about lock acquisition for holding() and debugging.
    lk->cpu = mycpu();
}

void release(struct spinlock* lk)
{
    assert(lk->locked);
    assert(holding(lk));

    lk->cpu = 0;

    // Tell the C compiler and the CPU to not move loads or stores
    // past this point, to ensure that all the stores in the critical
    // section are visible to other CPUs before the lock is released,
    // and that loads in the critical section occur strictly before
    // the lock is released.
    __sync_synchronize();

    lk->locked = 0;
    //__sync_lock_release(&lk->locked, __ATOMIC_RELEASE);

    pop_off();
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void push_off(void)
{
    int old = intr_get();

    intr_off();
    struct cpu* c = mycpu();
    if (c->noff == 0)
        c->intena = old;
    c->noff += 1;
}

void pop_off(void)
{
    struct cpu* c = mycpu();
    assert(!intr_get());        // no interrupt
    assert(c->noff > 0);
    c->noff -= 1;
    if (c->noff == 0 && c->intena)
        intr_on();
}

// Check whether this cpu is holding the lock.
int holding(struct spinlock* lk)
{
    int r;
    r = (lk->locked && lk->cpu == mycpu());
    return r;
}