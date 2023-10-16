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

// spinlock.c
void            acquire(struct spinlock* lk);
void            release(struct spinlock* lk);

// string.c
int             memcmp(const void* v1, const void* v2, uint n);
void*           memcpy(void* dst, const void* src, uint n);
void*           memmove(void* dst, const void* src, uint n);
void*           memset(void* dst, int c, uint n);
int             strlen(const char* s);

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
int             mappages(uint64* pgdir, uint64 vaddr, uint64 paddr, uint64 size, int perm);
uint64*         walk(uint64* pgdir, uint64 vaddr, int alloc);

#define         assert(x)                                           \
({                                                                  \
    if (!(x))                                                       \
    {                                                               \
        check_assertion();                                          \
        cprintf("%s:%d: assertion failed at cpu %d.\n", __FILE__, __LINE__, cpuid());  \
        for (;;);                                                   \
    }                                                               \
})

#endif