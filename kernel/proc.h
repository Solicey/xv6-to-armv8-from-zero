#ifndef __KERNEL_PROC_H
#define __KERNEL_PROC_H

#include "types.h"
#include "virt.h"
#include "spinlock.h"
#include "file.h"

#define NPROC               64
#define PROC_KSTACK_SIZE    PG_SIZE

struct trapframe
{
    uint64 x0;
    uint64 x1;
    uint64 x2;
    uint64 x3;
    uint64 x4;
    uint64 x5;
    uint64 x6;
    uint64 x7;
    uint64 x8;
    uint64 x9;
    uint64 x10;
    uint64 x11;
    uint64 x12;
    uint64 x13;
    uint64 x14;
    uint64 x15;
    uint64 x16;
    uint64 x17;
    uint64 x18;
    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 x29;
    uint64 x30;	        // user mode lr
    uint64 sp;          // user mode sp
    uint64 pc;          // user mode pc (elr)
    uint64 spsr;
};

struct context
{
    uint64 x16;
    uint64 x17;
    uint64 x18;
    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 x29;
    uint64 lr;
    uint64 sp;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc
{
    struct spinlock lock;

    enum procstate state;       // Process state
    void* chan;                 // ? If non-zero, sleeping on chan
    int killed;                 // If non-zero, have been killed
    int xstate;                 // Exit status to be returned to parent's wait
    int pid;                    // Process ID

    // wait_lock must be held when using this:
    struct proc* parent;        // Parent process

    // these are private to the process, so p->lock need not be held.
    char* kstack;               // Virtual address of kernel stack
    uint64 size;                // Size of process memory (bytes)
    uint64* pagetable;          // User page directory table
    struct trapframe* trapframe;// Trapframe for current syscall
    struct context context;     // swtch() here to run process
    struct file* ofile[NOFILE]; // Open files
    struct inode* cwd;          // Current directory
    char name[16];              // Process name (debugging)
};

struct cpu
{
    struct proc* proc;          // The process running on this cpu, or null.
    struct context context;     // swtch() here to enter scheduler().
    int noff;                   // Depth of push_off() nesting.
    int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

#endif