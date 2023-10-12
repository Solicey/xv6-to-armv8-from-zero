//
// a definition of data structure/methods
//
#ifndef __KERNEL_DEFS_H
#define __KERNEL_DEFS_H

#include "types.h"

// console.c
void            consoleinit(void);
void            cprintf(const char *fmt, ...);
void            panic(const char *fmt, ...);

// string.c
int             memcmp(const void* v1, const void* v2, uint n);
void*           memcpy(void* dst, const void* src, uint n);
void*           memmove(void* dst, const void* src, uint n);
void*           memset(void* dst, int c, uint n);
int             strlen(const char* s);

// uart.c
void            uartinit(void);
void            uartputc(char c);
void            uartputs(const char* s);



#endif