//
// a definition of data structure/methods
//
#ifndef __KERNEL_DEFS_H
#define __KERNEL_DEFS_H

#include "types.h"
#include "proc.h"
#include "spinlock.h"
#include "arm.h"
#include "sleeplock.h"
#include "buf.h"

// bio.c
void            binit(void);
struct buf*     bread(uint dev, uint blockno);
void            brelse(struct buf* b);
void            bwrite(struct buf* b);

// console.c
void            check_assertion(void);
void            consoleinit(void);
void            cprintf(const char *fmt, ...);
//void          panic(const char *fmt, ...);

// fs.c
void            fsinit(int dev);

// gic.c
void            gicinit(void);
void            irqhandle(struct trapframe* f, uint32 el);
void            intrset(int id, intrhandler ih);

// kalloc.c
void*           kalloc(void);
void            kinit(void);
void            kfree(void* paddr);

// proc.c
struct cpu*     mycpu(void);
struct proc*    myproc(void);
void            procinit(void);
void            scheduler(void);
void            sleep(void* chan, struct spinlock* lk);
void            userinit(void);
void            wakeup(void* chan);
void            yield(void);

// sleeplock.c
void            acquiresleep(struct sleeplock* lk);
int             holdingsleep(struct sleeplock* lk);
void            initsleeplock(struct sleeplock* lk, char* name);
void            releasesleep(struct sleeplock* lk);

// spinlock.c
void            acquire(struct spinlock* lk);
int             holding(struct spinlock* lk);
void            initlock(struct spinlock* lk, char* name);
void            pop_off(void);
void            push_off(void);
void            release(struct spinlock* lk);

// string.c
int             memcmp(const void* v1, const void* v2, uint n);
void*           memcpy(void* dst, const void* src, uint n);
void*           memmove(void* dst, const void* src, uint n);
void*           memset(void* dst, int c, uint n);
char*           safestrcpy(char* s, const char* t, int n);
int             strlen(const char* s);

// swtch.S
void            swtch(struct context* old, struct context* new);

// syscall.c
void            argaddr(int n, uint64* ip);
void            argint(int n, int* ip);
int             argstr(int n, char* buf, int max);
int             fetchaddr(uint64 addr, uint64* ip);
int             fetchstr(uint64 addr, char* buf, int max);
void            syscall(void);

// timer.c
void            timerinit(void);
void            timerintr(struct trapframe* f, int id, uint32 el);

// uart.c
void            uartsti(void);
void            uartinit(void);
void            uartintr(struct trapframe* f, int id, uint32 el);
void            uartputc(char c);
void            uartputs(const char* s);

// virtio_disk.c
void            virtio_disk_init(void);
void            virtio_disk_intr(struct trapframe* f, int id, uint32 el);
void            virtio_disk_rw(struct buf *b, int write);

// vm.c
int             copyin(uint64* pde, char* dst, uint64 srcvaddr, uint64 len);
int             copyinstr(uint64* pde, char* dst, uint64 srcvaddr, uint64 max);
int             mappages(uint64* pde, uint64 vaddr, uint64 paddr, uint64 size, int perm);
uint64*         uvmcreate(void);
void            uvmfirst(uint64* pde, char* src, uint size);
void            uvmfree(uint64* pde, uint64 size);
void            uvmswitch(struct proc* p);
void            uvmunmap(uint64* pde, uint64 vaddr, uint64 npages, int do_free);
uint64*         walk(uint64* pde, uint64 vaddr, int alloc);

#define         assert(x)                                           \
{                                                                   \
    if (!(x))                                                       \
    {                                                               \
        check_assertion();                                          \
        cprintf("%s:%d: assertion failed at cpu %d.\n",             \
        __FILE__, __LINE__, cpuid());                               \
        for (;;);                                                   \
    }                                                               \
}

#define         asserts(x, s)                                       \
{                                                                   \
    if (!(x))                                                       \
    {                                                               \
        check_assertion();                                          \
        cprintf("%s:%d: assertion failed at cpu %d: %s\n",          \
        __FILE__, __LINE__, cpuid(), s);                            \
        for (;;);                                                   \
    }                                                               \
}

#define         panic(s)                                            \
{                                                                   \
    check_assertion();                                              \
    cprintf("%s:%d: panicked at cpu %d: %s\n",                      \
    __FILE__, __LINE__, cpuid(), s);                                \
    for (;;);                                                       \
}

// number of elements in fixed-size array
#define NELEM(x)    (sizeof(x) / sizeof((x)[0]))

#endif