//
// a definition of data structure/methods
//
#ifndef __KERNEL_DEFS_H
#define __KERNEL_DEFS_H

#include "types.h"
#include "proc.h"
#include "spinlock.h"
#include "arm.h"

// console.c
void            check_assertion(void);
void            consoleinit(void);
void            cprintf(const char *fmt, ...);
//void          panic(const char *fmt, ...);

// gic.c
void            gicinit(void);
void            irqhandle(struct trapframe* f);
void            irqhset(int id, irqhandler ih);

// kalloc.c
void*           kalloc(void);
void            kinit(void);
void            kfree(void* paddr);

// proc.c
struct cpu*     mycpu(void);
struct proc*    myproc(void);
void            procinit(void);
void            scheduler(void);
void            userinit(void);
void            yield(void);

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
void            timerirqh(struct trapframe* f, int id);

// uart.c
void            uartintr(void);
void            uartinit(void);
void            uartirqh(struct trapframe* f, int id);
void            uartputc(char c);
void            uartputs(const char* s);

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
({                                                                  \
    if (!(x))                                                       \
    {                                                               \
        check_assertion();                                          \
        cprintf("%s:%d: assertion failed at cpu %d.\n", __FILE__, __LINE__, cpuid());  \
        for (;;);                                                   \
    }                                                               \
})

// number of elements in fixed-size array
#define NELEM(x)    (sizeof(x) / sizeof((x)[0]))

#endif