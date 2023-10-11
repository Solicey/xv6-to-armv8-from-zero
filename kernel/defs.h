//
// a definition of data structure/methods
//
#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

// console.c
void            consoleinit(void);
void            cprintf(const char *fmt, ...);
void            panic(const char *fmt, ...);

// uart.c
void            uartinit(void);
void            uartputc(char c);
void            uartputs(const char* s);



#endif