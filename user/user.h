#ifndef __USER_USER_H
#define __USER_USER_H

#include "kernel/types.h"
#include "kernel/stat.h"

// system calls
int exec(const char* path, char** argv);
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
char* sbrk(int n);
int fstat(int fd, struct stat* stat);
int mkdir(const char* path);
int unlink(const char* path);
int sleep(int t);
int pipe(int* fdarray);
int getpid(void);

// ulib.c
void printf(const char*, ...);
void fprintf(int fd, const char* fmt, ...);
char* strcpy(char* s, const char* t);
int strcmp(const char* p, const char* q);
uint strlen(const char* s);
void* memset(void* dst, int c, uint n);
char* strchr(const char* s, char c);
char* gets(char* buf, int max);
void* malloc(uint nbytes);
void free(void* ap);
void* memmove(void* vdst, const void* vsrc, int n);
int stat(const char* n, struct stat* st);
int atoi(const char* s);

#endif