#ifndef __KERNEL_SYSREGS_H
#define __KERNEL_SYSREGS_H

/* SCR_EL3, Secure Configuration Register (EL3). */
#define SCR_RESERVED                    (3 << 4)
#define SCR_RW                          (1 << 10)
#define SCR_HCE                         (1 << 8)
#define SCR_SMD                         (1 << 7)
#define SCR_NS                          (1 << 0)
#define SCR_VALUE                       (SCR_RESERVED | SCR_RW | SCR_HCE | SCR_SMD | SCR_NS)

/* SPSR_EL1/2/3, Saved Program Status Register. */
#define SPSR_MASK_ALL                   (7 << 6)
// M[3:2] is set to the value of PSTATE.EL on taking an exception to EL1 and copied to PSTATE.EL on executing an exception return operation in EL1.
// M[1] is unused and is 0 for all non-reserved values.
// M[0] is set to the value of PSTATE.SP on taking an exception to EL1 and copied to PSTATE.SP on executing an exception return operation in EL1.
#define SPSR_EL1h                       (5 << 0)
#define SPSR_EL2h                       (9 << 0)
#define SPSR_EL3_VALUE                  (SPSR_MASK_ALL | SPSR_EL2h)
#define SPSR_EL2_VALUE                  (SPSR_MASK_ALL | SPSR_EL1h)

/* HCR_EL2, Hypervisor Configuration Register (EL2). */
#define HCR_RW                          (1 << 31)
#define HCR_VALUE                       HCR_RW

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

// PSTATE
#define DAIF_I                          (1 << 7)

/* Translation Control Register */
#define TCR_T0SZ                        (64 - 48)               // top 32 bits must be either all 0 or all 1
#define TCR_T1SZ                        ((64 - 48) << 16)
#define TCR_TG0_4K                      (0 << 14)               // 4K page table
#define TCR_TG1_4K                      (0 << 30)
#define TCR_TBI0_IGNORED                (1 << 37)               // ttbr0 tagged address available
#define TCR_VALUE                       (TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K | TCR_TBI0_IGNORED)

#endif