//
// a definition of data structure/methods
//
#ifndef __KERNEL_DEFS_H
#define __KERNEL_DEFS_H

#include "types.h"
#include "proc.h"

// console.c
void            consoleinit(void);
void            cprintf(const char *fmt, ...);
void            panic(const char *fmt, ...);

// gic.c
void            gicinit(void);
void            irqhandle(struct trapframe* f);
void            irqhset(int id, irqhandler ih);

// kalloc.c
void*           kalloc(void);
void            kinit(void);
void            kfree(void* paddr);

// string.c
int             memcmp(const void* v1, const void* v2, uint n);
void*           memcpy(void* dst, const void* src, uint n);
void*           memmove(void* dst, const void* src, uint n);
void*           memset(void* dst, int c, uint n);
int             strlen(const char* s);

// timer.c
void            timerinit(void);
void            timerirqh(struct trapframe* f, int id);

#include "types.h"

// kalloc.c
void*           kalloc(void);
void            kfree(void*);
void            kinit(void);

// string.c
void*           memset(void*, int, uint);

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
        cprintf("%s:%d: assertion failed.\n", __FILE__, __LINE__);  \
        for (;;);                                                   \
    }                                                               \
})


#endif