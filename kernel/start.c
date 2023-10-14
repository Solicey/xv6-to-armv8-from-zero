#include "arm.h"
#include "defs.h"
#include "mmu.h"
#include "arm.h"

extern char edata[], ebss[], vectors[];
extern uint64 kpgdir[];

void start()
{
    // clear bss
    memset((void*)edata, 0, ebss - edata);

    consoleinit();
    kinit();

    lvbar(vectors);
    gicinit();
    uartintr();
    timerinit();
    sti();

    //asm volatile("swi #0");

    //mappages(kpgdir, 0x0000000047000000, 0x46000000, 0x20000, AP_KERNEL);

    for (;;);
=======
#include "types.h"
#include "param.h"
#include "memlayout.h"

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[PG_SIZE * NCPU];

// kernel.ld
/*extern uint64 kernel_pgtbl;
extern uint64 user_pgtbl;
extern uint64 kernel_l2_pgtbl;
extern uint64 user_l2_pgtbl;

uint64* k_l1 = &kernel_pgtbl;
uint64* u_l1 = &user_pgtbl;*/

extern char end[];
extern void main();

/*void set_boot_pgtbl(uint64 vaddr, uint64 paddr, uint32 len, int is_device_memory)
{
    for (int i = 0; i < len; i += PMD_SIZE)
    {
        int pgd_idx = PGD_IDX(vaddr);
        int pmd_idx = PMD_IDX(vaddr);
        uint64 pde = paddr & PMD_MASK;

        if (!is_device_memory)
        {
            pde |= ACCESS_FLAG | SH_IN_SH | AP_RW_1 | NON_SECURE_PA | MEM_ATTR_IDX_4 | ENTRY_BLOCK | ENTRY_VALID | UXN;
        }
        else
        {
            pde |= ACCESS_FLAG | AP_RW_1 | MEM_ATTR_IDX_0 | ENTRY_BLOCK | ENTRY_VALID;
        }
    }
}*/

void start()
{
    _putstr("starting xv6 on armv8...\n");

    uint32 val32;
    // read the main id register to make sure we are running on ARMv6
    asm("mrs %[r], midr_el1" : [r] "=r" (val32) : : );

    if (val32 >> 24 == 0x41)
    {
        _putstr("implementer: arm limited\n");
    }

    char arch = (val32 >> 16) & 0x0f;
    if ((arch != 7) && (arch != 0xf))
    {
        _putstr("need aarm v6 or higher\n");
    }

    //EL?
    asm("mrs %[r], currentel": [r] "=r" (val32) : : );

    val32 = (val32 & 0x0c) >> 2;
    switch (val32)
    {
    case 0:
        _putstr("current el: el0\n");
        break;
    case 1:
        _putstr("current el: el1\n");
        break;
    case 2:
        _putstr("current el: el2\n");
        break;
    case 3:
        _putstr("current el: el3\n");
        break;
    default:
        _putstr("current el: unknown\n");
    }

    //_putint("free mem start from: 0x", (uint64)end, "\n");
    main();

    // set PGD entries
    /*uint64 k_l2 = (uint64)&kernel_l2_pgtbl;
    for (int i = 0; i < 4; i++)
    {
        k_l1[i] = (k_l2 | ENTRY_TABLE | ENTRY_VALID);
        k_l2 += PGSIZE;
    }

    uint64 u_l2 = (uint64)&user_l2_pgtbl;
    for (int i = 0; i < 4; i++)
    {
        u_l1[i] = (u_l2 | ENTRY_TABLE | ENTRY_VALID);
        u_l2 += PGSIZE;
    }

    // prerequisite to enable paging

    // double map
    // V:0x0000_0000_4000_0000 ==> P:0x0000_0000_4000_0000
    set_boot_pgtbl((uint64)PHY_BASE, (uint64)PHY_BASE, INIT_KERN_SIZE, 0);
    // V:0xFFFF_FFFF_4000_0000 ==> P:0x0000_0000_4000_0000
    set_boot_pgtbl((uint64)KERN_BASE + (uint64)PHY_BASE, (uint64)PHY_BASE, INIT_KERN_SIZE, 0);
    */
}