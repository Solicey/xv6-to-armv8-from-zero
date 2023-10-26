#ifndef __USER_USER_H
#define __USER_USER_H

#include "kernel/types.h"

// system calls
int exit(int) __attribute__((noreturn));
int write(int fd, const void* p, int n);
int open(const char* path, int omode);
int mknod(const char* path, short major, short minor);
int dup(int fd);
int fork(void);
int wait(int* addr);
int yield(void);

// ulib.c
void printf(const char*, ...);

#endif