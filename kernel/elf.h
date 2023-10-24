#ifndef __KERNEL_ELF_H
#define __KERNEL_ELF_H

#include "types.h"
// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr
{
    uint magic;         // must equal ELF_MAGIC
    uchar elf[12];
    ushort type;
    ushort machine;
    uint version;
    uint64 entry;       /* Entry point virtual address */
    uint64 phoff;       /* Program header table file offset */
    uint64 shoff;       /* Section header table file offset */
    uint flags;
    ushort ehsize;      /* ELF header size in bytes */
    ushort phentsize;   /* Program header table entry size */
    ushort phnum;       /* Program header table entry count */
    ushort shentsize;   /* Section header table entry size */
    ushort shnum;       /* Section header table entry count */
    ushort shstrndx;    /* Section header string table index */
};

// Program section header
struct proghdr
{
    uint32 type;        /* Segment type */
    uint32 flags;       /* Segment flags */
    uint64 off;         /* Segment file offset */
    uint64 vaddr;       /* Segment virtual address */
    uint64 paddr;       /* Segment physical address */
    uint64 filesz;      /* Segment size in file */
    uint64 memsz;       /* Segment size in memory */
    uint64 align;       /* Segment alignment */
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

#endif