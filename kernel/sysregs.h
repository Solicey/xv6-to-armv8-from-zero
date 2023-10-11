#ifndef __KERNEL_SYSREGS_H
#define __KERNEL_SYSREGS_H

/* SCTLR_EL1, System Control Register (EL1) */
#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0)
#define SCTLR_MMU_ENABLED               (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED        (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

/* ESR_EL1, Exception Syndrome Register (EL1) */
#define ESR_ELx_EC_SHIFT                26
#define ESR_ELx_EC_SVC64                0x15
#define ESR_ELx_EC_DABT_LOW             0x24

#endif