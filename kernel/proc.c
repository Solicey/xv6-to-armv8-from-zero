#include "proc.h"
#include "types.h"
#include "defs.h"
#include "arm.h"
#include "virt.h"
#include "spinlock.h"
#include "memlayout.h"
#include "param.h"

extern char eentry[];

struct cpu cpus[NCPU];
struct proc proc[NPROC];

struct proc* initproc;

int nextpid = 1;
struct spinlock pid_lock;

void forkret(void);
extern void trapret(void);

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

struct cpu* mycpu(void)
{
    int id = cpuid();
    struct cpu* c = &cpus[id];
    return c;
}

// Return the current struct proc *, or zero if none.
struct proc* myproc(void)
{
    push_off();
    struct cpu *c = mycpu();
    struct proc *p = c->proc;
    pop_off();
    return p;
}

void procinit(void)
{
    struct proc* p;

    initlock(&pid_lock, "nextpid");
    //initlock(&wait_lock, "wait_lock");

    for (p = proc; p < &proc[NPROC]; p++)
    {
        initlock(&p->lock, "proc");
        p->state = UNUSED;
        //p->kstack = KSTACK((int)(p - proc));
    }
}

int allocpid()
{
    int pid;

    acquire(&pid_lock);
    pid = nextpid;
    nextpid = nextpid + 1;
    release(&pid_lock);

    return pid;
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
/*uint64* proc_pagetable(struct proc* p)
{
    uint64* pde;

    pde = uvmcreate();
    if (pde == NULL)
        return NULL;

    // map the trampoline code (for system call return)
    // at the highest user virtual address.
    //if (mappages(pde, TRAMPOLINE, ))
}*/

/*void proc_freepagetable(uint64* pde, uint64 size)
{
    uvmunmap(pde, TRAMPOLINE, 1, 0);
    uvmunmap(pde, TRAPFRAME, 1, 0);
    uvmfree(pde, size);
}*/

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
/*static void freeproc(struct proc* p)
{
    if (p->trapframe)
        kfree((void*)p->trapframe);
    p->trapframe = NULL;

    if (p->pagetable)
        proc_freepagetable(p->pagetable, p->sz);
    p->pagetable = 0;
    p->sz = 0;
    p->pid = 0;
    p->parent = NULL;
    p->name[0] = 0;
    p->chan = 0;
    p->killed = 0;
    p->xstate = 0;
    p->state = UNUSED;
}*/

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc* allocproc(void)
{
    struct proc* p;
    char* sp;

    for (p = proc; p < &proc[NPROC]; p++)
    {
        acquire(&p->lock);
        if (p->state == UNUSED)
            goto found;
        else
            release(&p->lock);
    }
    return NULL;

found:
    p->pid = allocpid();
    p->state = USED;

    if ((p->kstack = (char*)kalloc()) == NULL)
    {
        p->state = UNUSED;
        release(&p->lock);
        return NULL;
    }
    cprintf("p->kstack: 0x%p\n", p->kstack);

    // Stack size is 4K.
    sp = p->kstack + PROC_KSTACK_SIZE;

    // Leave room for trap frame.
    sp -= sizeof(*(p->trapframe));
    p->trapframe = (struct trampframe*)sp;

    // trapret
    sp -= sizeof(uint64);
    *(uint64*)sp = (uint64)trapret;
    cprintf("sp: 0x%p\n", sp);
    cprintf("trapret: 0x%p\n", *(uint64*)sp);
    // frame pointer
    sp -= sizeof(uint64);
    *(uint64*)sp = (uint64)p->kstack + PROC_KSTACK_SIZE;
    cprintf("sp: 0x%p\n", sp);
    cprintf("fp: 0x%p\n", *(uint64*)sp);
    // context is not store on proc kernel stack

    // skip the push {fp, lr} instruction in the prologue of forkret.
    // This is different from x86, in which the harderware pushes return
    // address before executing the callee. In ARM, return address is
    // loaded into the lr register, and push to the stack by the callee
    // (if and when necessary). We need to skip that instruction and let
    // it use our implementation.
    p->context.lr = (uint64)forkret + 8;
    p->context.sp = (uint64)sp;

    release(&p->lock);
    return p;
}

// Set up first user process.
// hand-craft the first user process. We link initcode.S into the kernel
// as a binary, the linker will generate _binary_initcode_start
void userinit(void)
{
    struct proc* p;
    extern char _binary_initcode_size[], _binary_initcode_start[];

    p = allocproc();
    initproc = p;

    assert(p->pagetable = kalloc());
    memset(p->pagetable, 0, PG_SIZE);

    uint64 initcode_s = (uint64)eentry + (uint64)_binary_initcode_start + KERN_BASE;
    cprintf("initcode_s: 0x%p\n", initcode_s);
    uvmfirst(p->pagetable, (char*)initcode_s, (uint64)_binary_initcode_size);

    p->sz = PROC_KSTACK_SIZE;

    memset(p->trapframe, 0, sizeof(*(p->trapframe)));
    p->trapframe->spsr = 0x0;
    p->trapframe->sp = PROC_KSTACK_SIZE;    // user stack
    p->trapframe->x30 = 0;                  // lr

    // Set the user pc. The actual pc loaded is in
    // p->tf, the trapframe.
    p->trapframe->pc = 0;                   // beginning of initcode.S

    safestrcpy(p->name, "initcode", sizeof(p->name));
    // TODO: cwd

    p->state = RUNNABLE;
}

// A fork child's very first scheduling by scheduler()
// will swtch here. "Return" to user space.
void forkret(void)
{
    static int first = 1;

    // ? Still holding p->lock from scheduler.
    release(&myproc()->lock);

    if (first)
    {
        first = 0;
        fsinit(ROOTDEV);
    }

    // go to trapret
    return;
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void)
{
    struct proc* p;
    struct cpu* c = mycpu();

    c->proc = NULL;
    for (;;)
    {
        // Avoid deadlock by ensuring that devices can interrupt.
        intr_on();

        for (p = proc; p < &proc[NPROC]; p++)
        {
            acquire(&p->lock);
            if (p->state == RUNNABLE)
            {
                // Switch to chosen process.  It is the process's job
                // to release its lock and then reacquire it
                // before jumping back to us.
                p->state = RUNNING;
                c->proc = p;
                uvmswitch(p);

                swtch(&c->context, &p->context);

                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = NULL;
            }
            release(&p->lock);
        }
    }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
    int intena;
    struct proc *p = myproc();

    assert(holding(&p->lock));
    assert(mycpu()->noff == 1);
    assert(p->state != RUNNING);
    assert(!intr_get());    // sched should not be interruptable

    intena = mycpu()->intena;
    swtch(&p->context, &mycpu()->context);
    mycpu()->intena = intena;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
/*void exit(int status)
{
    struct proc* p = myproc();

}*/

// Give up the CPU for one scheduling round.
void yield(void)
{
    struct proc* p = myproc();
    cprintf("proc %d yield!\n", p->pid);
    acquire(&p->lock);
    p->state = RUNNABLE;
    sched();
    release(&p->lock);
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void* chan, struct spinlock* lk)
{
    struct proc* p = myproc();

    // Must acquire p->lock in order to
    // change p->state and then call sched.
    // Once we hold p->lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup locks p->lock),
    // so it's okay to release lk.
    acquire(&p->lock);
    release(lk);

    // Go to sleep.
    p->chan = chan;
    p->state = SLEEPING;

    sched();

    // Tidy up.
    p->chan = NULL;

    // Reacquire original lock.
    release(&p->lock);
    acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void wakeup(void* chan)
{
    struct proc* p;

    for (p = proc; p < &proc[NPROC]; p++)
    {
        if (p != myproc())
        {
            acquire(&p->lock);
            if (p->state == SLEEPING && p->chan == chan)
            {
                p->state = RUNNABLE;
            }
            release(&p->lock);
        }
    }
}