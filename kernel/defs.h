//
// a definition of data structure/methods
//
#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#include "types.h"

// kalloc.c
void*           kalloc(void);
void            kfree(void*);
void            kinit(void);

// string.c
void*           memset(void*, int, uint);

// uart.c
void            _putint(char*, uint64, char*);
void            _putstr(char*);
void            uartinit();

// vm.c
void            kvminit(void);
void            kvminithart(void);
void            kvmmap(ppte, uint64, uint64, uint64, int, int, int);
int             mappages(ppte, uint64, uint64, uint64, int, int, int);
ppte            walk(ppte, uint64, int, int);

#endif