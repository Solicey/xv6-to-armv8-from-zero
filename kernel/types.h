#ifndef __KERNEL_TYPES_H
#define __KERNEL_TYPES_H

struct trapframe;

typedef unsigned int        uint;
typedef unsigned short      ushort;
typedef unsigned char       uchar;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef char                int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;

#ifndef NULL
#define NULL                0
#endif

typedef void (*intrhandler)  (struct trapframe* tf, int id, uint32 el);

#endif