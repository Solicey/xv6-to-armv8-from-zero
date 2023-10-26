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
int read(int fd, void* p, int n);
int chdir(const char* path);
int close(int fd);

// ulib.c
void printf(const char*, ...);
char* strcpy(char* s, const char* t);
int strcmp(const char* p, const char* q);
uint strlen(const char* s);
void* memset(void* dst, int c, uint n);
char* strchr(const char* s, char c);
char* gets(char* buf, int max);

#endif