#ifndef __KERNEL_MMU_H
#define __KERNEL_MMU_H

#define PG_SIZE                 4096
#define PG_OFFSET               12
#define PG_ROUND_UP(x)          (((x) + PG_SIZE - 1) & ~(PG_SIZE - 1))
#define PG_ROUND_DOWN(x)        ((x) & ~(PG_SIZE - 1))
#define PG_ADDR_MASK            0xfffffffff000

// Access permission
#define AP_KERNEL               (0 << 6)
#define AP_USER                 (1 << 6)
#define AP_RW                   (0 << 7)
#define AP_RO                   (1 << 7)
#define AP_MASK                 (3 << 6)

// Access flags
#define AF_USED                 (1 << 10)

// Memory region attributes 
#define MT_DEVICE_nGnRnE        0x0
#define MT_NORMAL_NC            0x1
#define MT_DEVICE_nGnRnE_FLAGS  0x00
#define MT_NORMAL_NC_FLAGS      0x44
#define MAIR_VALUE              (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))

#define MM_TYPE_VALID           0x1
#define MM_TYPE_BLOCK           0x1
#define MM_TYPE_TABLE           0x3
#define MM_TYPE_PAGE            0x3
#define MM_TYPE_MASK            0x3

// PDE flags 
#define PDE_PAGE                (MM_TYPE_BLOCK | (MT_NORMAL_NC << 2) | AP_KERNEL | AF_USED)
#define PDE_DEVICE              (MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | AP_KERNEL | AF_USED)
#define PDE_TABLE               MM_TYPE_TABLE

// PTE flags 
// Access permission not included
#define PTE_PAGE                (MM_TYPE_PAGE | (MT_NORMAL_NC << 2) | AF_USED)

// extract the three 9-bit page table indices from a virtual address.
#define PX_MASK                 0x1ff // 9 bits
#define PX_OFFSET(level)        (PG_OFFSET + (9 * (3 - level)))
#define PX(level, vaddr)        ((((uint64)(vaddr)) >> PX_OFFSET(level)) & PX_MASK)
#define PX_ENTRY_CNT            512

#endif