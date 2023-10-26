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
#include "fs.h"
#include "stat.h"
#include "file.h"

// bio.c
void            binit(void);
void            bpin(struct buf *b);
struct buf*     bread(uint dev, uint blockno);
void            brelse(struct buf* b);
void            bunpin(struct buf *b);
void            bwrite(struct buf* b);

// console.c
void            check_assertion(void);
void            consoleinit(void);
void            consoleintr(int c);
int             consoleread(int user_dst, uint64 dst, int n);
int             consolewrite(int user_src, uint64 src, int n);
void            consputc(int c);
//void          panic(const char *fmt, ...);

// exec.c
int             exec(char *path, char **argv);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file* f);
struct file*    filedup(struct file* f);
void            fileinit(void);
int             fileread(struct file* f, uint64 addr, int n);
int             filestat(struct file* f, uint64 addr);
int             filewrite(struct file* f, uint64 addr, int n);

// fs.c
int             dirlink(struct inode* dp, char* name, uint inum);
struct inode*   dirlookup(struct inode* dp, char* name, uint* poff);
void            fsinit(int dev);
struct inode*   ialloc(uint dev, short type);
struct inode*   idup(struct inode *ip);
void            iinit(void);
void            ilock(struct inode* ip);
void            iput(struct inode* ip);
void            itrunc(struct inode* ip);
void            iunlock(struct inode* ip);
void            iunlockput(struct inode* ip);
void            iupdate(struct inode *ip);
int             namecmp(const char* s, const char* t);
struct inode*   namei(char* path);
struct inode*   nameiparent(char* path, char* name);
int             readi(struct inode* ip, int user_dst, uint64 dst, uint off, uint n);
void            stati(struct inode* ip, struct stat* st);
int             writei(struct inode* ip, int user_src, uint64 src, uint off, uint n);

// gic.c
void            gicinit(void);
void            irqhandle(struct trapframe* f, uint32 el);
void            intrset(int id, intrhandler ih);

// kalloc.c
void*           kalloc(void);
void            kinit(void);
void            kfree(void* paddr);

// log.c
void            begin_op(void);
void            end_op(void);
void            initlog(int dev, struct superblock *sb);
void            log_write(struct buf *b);

// printf.c
void            printf(const char *fmt, ...);
void            printfinit(void);

// proc.c
int             either_copyin(void* dst, int user_src, uint64 src, uint64 len);
int             either_copyout(int user_dst, uint64 dst, void* src, uint64 len);
void            exit(int status);
int             fork(void);
int             killed(struct proc *p);
struct cpu*     mycpu(void);
struct proc*    myproc(void);
void            procinit(void);
void            sched(void);
void            scheduler(void);
void            sleep(void* chan, struct spinlock* lk);
void            userinit(void);
int             wait(uint64 addr);
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
int             strncmp(const char* p, const char* q, uint n);
char*           strncpy(char* s, const char* t, int n);

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
int             uartgetc(void);
void            uartinit(void);
void            uartintr(struct trapframe* f, int id, uint32 el);
void            uartputc(int c);
void            uartputc_sync(int c);
void            uartstart(void);
void            uartsti(void);
void            _uartputs(const char* s);

// virtio_disk.c
void            virtio_disk_init(void);
void            virtio_disk_intr(struct trapframe* f, int id, uint32 el);
void            virtio_disk_rw(struct buf *b, int write);

// vm.c
int             copyin(uint64* pde, char* dst, uint64 srcvaddr, uint64 len);
int             copyinstr(uint64* pde, char* dst, uint64 srcvaddr, uint64 max);
int             copyout(uint64* pde, uint64 dstvaddr, char *src, uint64 len);
int             mappages(uint64* pde, uint64 vaddr, uint64 paddr, uint64 size, int perm);
uint64          uvmalloc(uint64* pde, uint64 oldsz, uint64 newsz);
void            uvmclear(uint64* pde, uint64 vaddr);
int             uvmcopy(uint64* old, uint64* new, uint64 size);
uint64*         uvmcreate(void);
void            uvmfirst(uint64* pde, char* src, uint size);
void            uvmfree(uint64* pde, uint64 size);
void            uvmswitch(struct proc* p);
void            uvmunmap(uint64* pde, uint64 vaddr, uint64 npages, int do_free);
uint64*         walk(uint64* pde, uint64 vaddr, int alloc);
uint64          walkaddr(uint64* pde, uint64 vaddr);

#define         assert(x)                                           \
{                                                                   \
    if (!(x))                                                       \
    {                                                               \
        check_assertion();                                          \
        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        printf("\n%s:%d: ASSERTION FAILED AT CPU %d.\n",            \
        __FILE__, __LINE__, cpuid());                               \
        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        for (;;);                                                   \
    }                                                               \
}

#define         asserts(x, s)                                       \
{                                                                   \
    if (!(x))                                                       \
    {                                                               \
        check_assertion();                                          \
        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        printf("\n%s:%d: ASSERTION FAILED AT CPU %d: %s\n",         \
        __FILE__, __LINE__, cpuid(), s);                            \
        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
        for (;;);                                                   \
    }                                                               \
}

#define         panic(s)                                            \
{                                                                   \
    check_assertion();                                              \
    printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
    printf("\n%s:%d: PANICKED AT CPU %d: %s\n",                     \
    __FILE__, __LINE__, cpuid(), s);                                \
    printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
    for (;;);                                                       \
}

// number of elements in fixed-size array
#define NELEM(x)    (sizeof(x) / sizeof((x)[0]))

#endif