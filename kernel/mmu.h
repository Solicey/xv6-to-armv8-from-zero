#ifndef __KERNEL_MMU_H
#define __KERNEL_MMU_H

#define PG_SIZE                 4096

/* Access permission */
#define AP_RW                   (0 << 6)      

/* Access flags */
#define AF_USED                 (1 << 10)

/* Memory region attributes */
#define MT_DEVICE_nGnRnE        0x0
#define MT_NORMAL_NC            0x1
#define MT_DEVICE_nGnRnE_FLAGS  0x00
#define MT_NORMAL_NC_FLAGS      0x44
#define MAIR_VALUE              (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))

#define MM_TYPE_BLOCK           0x1
#define MM_TYPE_TABLE           0x3
#define MM_TYPE_PAGE            0x3

/* PTE flags */
#define PDE_PAGE                (MM_TYPE_BLOCK | (MT_NORMAL_NC << 2) | AP_RW | AF_USED)
#define PDE_DEVICE              (MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | AP_RW | AF_USED)
#define PDE_TABLE               MM_TYPE_TABLE

/* Translation Control Register */
#define TCR_T0SZ                (64 - 48)               // top 32 bits must be either all 0 or all 1
#define TCR_T1SZ                ((64 - 48) << 16)
#define TCR_TG0_4K              (0 << 14)               // 4K page table
#define TCR_TG1_4K              (0 << 30)
#define TCR_TBI0_IGNORED        (1 << 37)               // ttbr0 tagged address available
#define TCR_VALUE               (TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K | TCR_TBI0_IGNORED)

#endif