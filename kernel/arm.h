#ifndef __KERNEL_ARM_H
#define __KERNEL_ARM_H

#include "types.h"

//

//

#define PG_SIZE             4096    // bytes per page
#define PG_OFFSET           12      // bits of offset within a page
#define PG_ROUND_UP(sz)     (((sz) + PG_SIZE - 1) & ~(PG_SIZE - 1))
#define PG_ROUND_DOWN(a)    (((a)) & ~(PG_SIZE - 1))

// shift a physical address to the right place for a PTE.
//#define PA2PTE(paddr)       ((((uint64)paddr) >> 12) << 10)
//#define PTE2PA(pte)         (((pte) >> 10) << 12)
//#define PTE_FLAGS(pte)      ((pte) & 0x3FF)

// extract the three 9-bit page table indices from a virtual address.
#define PX_MASK             0x1ff // 9 bits
#define PX_OFFSET(level)    (PG_OFFSET + (9 * (level)))
#define PX_SIZE(level)      (1 << PX_OFFSET(level))
#define PX(level, vaddr)    ((((uint64)(vaddr)) >> PX_OFFSET(level)) & PX_MASK)
#define PX_ADDR_MASK        0xfffffffff000	// bit 47 - bit 12
#define PX_ROUND_DOWN(a, level)     (((a)) & ~(PX_SIZE(level) - 1))

#define V2P(a)              (((uint64)(a)) - (uint64)KERN_BASE)
#define P2V(a)              (((void *)(a)) + (uint64)KERN_BASE)

// level 1 (1GB) pde
#define PGD_OFFSET      30
#define PGD_SIZE        (1 << PGD_OFFSET)
#define PGD_MASK        (~(PGD_SIZE - 1))
#define ENTRY_PER_PGD   4
#define PGD_IDX(vaddr)  (((uint64)(vaddr) >> PGD_OFFSET) & (ENTRY_PER_PGD - 1))

// level 2 (2MB) pde
#define PMD_OFFSET      21
#define PMD_SIZE        (1 << PMD_OFFSET)
#define PMD_MASK        (~(PMD_SIZE - 1))
#define ENTRY_PER_PMD   512
#define PMD_IDX(vaddr)  (((uint64)(vaddr) >> PMD_OFFSET) & (ENTRY_PER_PMD - 1))

// level 3 (4KB) pte
#define PTE_OFFSET      12
#define PTE_SIZE        (1 << PTE_OFFSET)
#define ENTRY_PER_PTE   512
#define PTE_IDX(vaddr)  (((uint64)(vaddr) >> PTE_OFFSET) & (ENTRY_PER_PTE - 1))

// block entry's attributes
// https://blog.csdn.net/dai_xiangjun/article/details/120138732

#define ENTRY_VALID	    0x01
#define ENTRY_BLOCK	    0x00
#define ENTRY_TABLE	    0x02
#define ENTRY_PAGE	    0x02
#define ENTRY_MASK	    0x03
#define ENTRY_FALUT	    0x00

// MAIR_ELn寄存器用来表示内存的属性，比如设备内存(Device Memory)、普通内存等
// 软件可以设置8个不同内存属性。常见的内存属性有:
// 0: DEVICE_nGnRnE
// 1: DEVICE_nGnRE
// 2: DEVICE_GRE
// 3: NORMAL_NC
// 4: NORMAL
// 5: NORMAL_WT
// AttrIndex用来索引不同的内存属性
#define MEM_ATTR_IDX_0	(0 << 2)
#define MEM_ATTR_IDX_1	(1 << 2)
#define MEM_ATTR_IDX_2	(2 << 2)
#define MEM_ATTR_IDX_3	(3 << 2)
#define MEM_ATTR_IDX_4	(4 << 2)
#define MEM_ATTR_IDX_5	(5 << 2)
#define MEM_ATTR_IDX_6	(6 << 2)
#define MEM_ATTR_IDX_7	(7 << 2)

// 非安全比特位(Non-secure)。当处于安全模式时用来指定访问的内存地址是安全映射的还是非安全映射的
#define NON_SECURE_PA   (1 << 5)

// 数据访问权限比特位
// AP[1]: 表示该内存允许用户权限(EL0)还是更高权限的特权异常等级(EL1)来访问
// 1: 表示可以被EL0以及更高特权的异常等级访问
// 0: 表示不能被EL0访问，但是可以被EL1访问
// AP[2]: 只读权限还是可读可写权限
// 1: 表示只读
// 0: 表示可读可写
#define AP_RW_1         (0 << 6)
#define AP_RW_1_0       (1 << 6)
#define AP_RO_1         (2 << 6)
#define AP_RO_1_0       (3 << 6)
#define AP_MASK         (3 << 6)

// https://blog.csdn.net/dai_xiangjun/article/details/120138732
// 内存缓存共享属性
// 00: 没有共享
// 01: 保留
// 10: Outer Shareable
// 11: Inner Shareable
#define SH_NON_SH       (0 << 8)
#define SH_UNPRED       (1 << 8)
#define SH_OUT_SH       (2 << 8)
#define SH_IN_SH        (3 << 8)

// 访问比特位(access flag), 当第一次访问页面时硬件会自动设置这个访问比特位
#define ACCESS_FLAG     (1 << 10)

// 该页面在特权模式下不能执行
#define PXN             (0x20000000000000)
// XN表示该页面在任何模式下都不能执行
// UXN表示该页面在用户模式下不能执行
#define UXN             (0x40000000000000)

#endif 