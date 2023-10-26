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
    initlock(&wait_lock, "wait_lock");

    for (p = proc; p < &proc[NPROC]; p++)
    {
        initlock(&p->lock, "proc");
        p->state = UNUSED;
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

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void freeproc(struct proc* p)
{
    if (p->kstack)
        kfree((void*)p->kstack);
    p->kstack = NULL;
    if (p->pagetable)
        uvmfree(p->pagetable, p->size);
    p->pagetable = NULL;

    p->size = 0;
    p->pid = 0;
    p->parent = NULL;
    p->name[0] = 0;
    p->chan = NULL;
    p->killed = 0;
    p->xstate = 0;
    p->state = UNUSED;
    p->trapframe = NULL;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
// Still holding lock when returned.
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
        freeproc(p);
        release(&p->lock);
        return NULL;
    }
    memset(p->kstack, 0, PG_SIZE);
    //printf("p->kstack: 0x%p\n", p->kstack);

    // Stack size is 4K.
    sp = p->kstack + PROC_KSTACK_SIZE;

    // Page table.
    if ((p->pagetable = (uint64*)kalloc()) == NULL)
    {
        freeproc(p);
        release(&p->lock);
        return NULL;
    }
    memset(p->pagetable, 0, PG_SIZE);

    // Leave room for trap frame.
    sp -= sizeof(*(p->trapframe));
    p->trapframe = (struct trampframe*)sp;

    // trapret
    sp -= sizeof(uint64);
    *(uint64*)sp = (uint64)trapret;
    //printf("sp: 0x%p\n", sp);
    //printf("trapret: 0x%p\n", *(uint64*)sp);
    // frame pointer
    sp -= sizeof(uint64);
    *(uint64*)sp = (uint64)p->kstack + PROC_KSTACK_SIZE;
    //printf("sp: 0x%p\n", sp);
    //printf("fp: 0x%p\n", *(uint64*)sp);
    // context does not store on proc kernel stack

    // skip the push {fp, lr} instruction in the prologue of forkret.
    // This is different from x86, in which the harderware pushes return
    // address before executing the callee. In ARM, return address is
    // loaded into the lr register, and push to the stack by the callee
    // (if and when necessary). We need to skip that instruction and let
    // it use our implementation.
    memset(&p->context, 0, sizeof(p->context));
    p->context.lr = (uint64)forkret + 8;
    p->context.sp = (uint64)sp;

    //release(&p->lock);
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

    uint64 initcode_s = (uint64)eentry + (uint64)_binary_initcode_start + KERN_BASE;
    //printf("initcode_s: 0x%p\n", initcode_s);
    uvmfirst(p->pagetable, (char*)initcode_s, (uint64)_binary_initcode_size);

    p->size = PROC_KSTACK_SIZE;

    memset(p->trapframe, 0, sizeof(*(p->trapframe)));
    p->trapframe->spsr = 0x0;
    p->trapframe->sp = PROC_KSTACK_SIZE;    // user stack
    p->trapframe->x30 = 0;                  // lr

    // Set the user pc. The actual pc loaded is in
    // p->tf, the trapframe.
    p->trapframe->pc = 0;                   // beginning of initcode.S

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");

    p->state = RUNNABLE;

    release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here. "Return" to user space.
void forkret(void)
{
    static int first = 1;

    // Still holding p->lock from scheduler.
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
                //printf("switch to: %d\n", p->pid);
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

    //printf("sched!\n");

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

    acquire(&p->lock);
    //printf("yield start %d\n", p->pid);
    if (p->state != RUNNING)
    {
        release(&p->lock);
        return;
    }
    p->state = RUNNABLE;
    sched();
    //printf("yield end %d\n", p->pid);
    release(&p->lock);
}

int killed(struct proc *p)
{
    int k;

    acquire(&p->lock);
    k = p->killed;
    release(&p->lock);
    return k;
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

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
// ! Might not need this because we have two page tables
int either_copyout(int user_dst, uint64 dst, void* src, uint64 len)
{
    //memmove((char*)dst, src, len);
    //return 0;

    struct proc* p = myproc();
    if (user_dst)
    {
        return copyout(p->pagetable, dst, src, len);
    }
    else
    {
        memmove((char*)dst, src, len);
        return 0;
    }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
// ! Might not need this because we have two page tables
int either_copyin(void* dst, int user_src, uint64 src, uint64 len)
{
    //memmove(dst, (char*)src, len);
    //return 0;

    struct proc* p = myproc();
    if (user_src)
    {
        return copyin(p->pagetable, dst, src, len);
    }
    else
    {
        memmove(dst, (char*)src, len);
        return 0;
    }
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc* p)
{
    struct proc *pp;

    for (pp = proc; pp < &proc[NPROC]; pp++)
    {
        if (pp->parent == p)
        {
            pp->parent = initproc;
            wakeup(initproc);
        }
    }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status)
{
    struct proc *p = myproc();

    if (p == initproc)
        panic("init exiting");

    // Close all open files.
    for (int fd = 0; fd < NOFILE; fd++)
    {
        if (p->ofile[fd])
        {
            struct file *f = p->ofile[fd];
            fileclose(f);
            p->ofile[fd] = 0;
        }
    }

    begin_op();
    iput(p->cwd);
    end_op();
    p->cwd = NULL;

    acquire(&wait_lock);

    // Give any children to init.
    reparent(p);

    // Parent might be sleeping in wait().
    wakeup(p->parent);

    acquire(&p->lock);

    p->xstate = status;
    p->state = ZOMBIE;

    release(&wait_lock);

    // Jump into the scheduler, never to return.
    sched();
    panic("zombie exit");
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void)
{
    int i, pid;
    struct proc *np;
    struct proc *p = myproc();

    // Allocate process.
    if ((np = allocproc()) == NULL)
    {
        return -1;
    }

    // Copy user memory from parent to child.
    if (uvmcopy(p->pagetable, np->pagetable, p->size) < 0)
    {
        freeproc(np);
        release(&np->lock);
        return -1;
    }
    np->size = p->size;
    //printf("fork size: 0x%x\n", np->size);

    // copy saved user registers.
    *(np->trapframe) = *(p->trapframe);

    //printf("fork trapframe spsr: 0x%x\n", np->trapframe->spsr);
    // Cause fork to return 0 in the child.
    np->trapframe->x0 = 0;

    // increment reference counts on open file descriptors.
    for (i = 0; i < NOFILE; i++)
        if (p->ofile[i])
            np->ofile[i] = filedup(p->ofile[i]);
    np->cwd = idup(p->cwd);

    safestrcpy(np->name, p->name, sizeof(p->name));

    pid = np->pid;

    release(&np->lock);

    // ? wait lock
    acquire(&wait_lock);
    np->parent = p;
    release(&wait_lock);

    acquire(&np->lock);
    np->state = RUNNABLE;
    release(&np->lock);

    return pid;
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr)
{
    struct proc *pp;
    int havekids, pid;
    struct proc *p = myproc();

    acquire(&wait_lock);

    for (;;)
    {
        // Scan through table looking for exited children.
        havekids = 0;
        for (pp = proc; pp < &proc[NPROC]; pp++)
        {
            if (pp->parent == p)
            {
                // make sure the child isn't still in exit() or swtch().
                acquire(&pp->lock);

                havekids = 1;
                if (pp->state == ZOMBIE)
                {
                    // Found one.
                    pid = pp->pid;
                    if (addr != 0 && copyout(p->pagetable, addr, (char*)&pp->xstate,
                        sizeof(pp->xstate)) < 0)
                    {
                        release(&pp->lock);
                        release(&wait_lock);
                        return -1;
                    }
                    freeproc(pp);
                    release(&pp->lock);
                    release(&wait_lock);
                    return pid;
                }
                release(&pp->lock);
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || killed(p))
        {
            release(&wait_lock);
            return -1;
        }

        // Wait for a child to exit.
        sleep(p, &wait_lock);  //DOC: wait-sleep
    }
}