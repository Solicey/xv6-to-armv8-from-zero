#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "arm.h"
#include "defs.h"

ppte kpgtbl;

extern char text_start[];
extern char text_end[];
extern char kernel_end[]; // kernel.ld sets this to end of kernel code.

// Initialize the one kernel_pagetable
// Make a direct-map page table for the kernel.
void kvminit(void)
{
    _putstr("kvminit begin!\n");
    kpgtbl = (ppte)kalloc();
    //_putint("kpgtbl: 0x", (uint64)kpgtbl, "\n");

    memset(kpgtbl, 0, PG_SIZE);

    // uart
    // add to prevent crash on _puts
    kvmmap(kpgtbl, UART0, UART0, DEV_MEM_SIZE, AP_RW_1, true, 1);
    kvmmap(kpgtbl, KERN_BASE + UART0, UART0, DEV_MEM_SIZE, AP_RW_1, true, 1);

    // gic
    // ?? why do I have to double map?
    kvmmap(kpgtbl, GIC, GIC, DEV_MEM_SIZE, AP_RW_1, true, 1);
    kvmmap(kpgtbl, KERN_BASE + GIC, GIC, DEV_MEM_SIZE, AP_RW_1, true, 1);

    // mmio
    kvmmap(kpgtbl, MMIO, MMIO, DEV_MEM_SIZE, AP_RW_1, true, 1);
    kvmmap(kpgtbl, KERN_BASE + MMIO, MMIO, DEV_MEM_SIZE, AP_RW_1, true, 1);

    // double map the low memory, required to enable paging

    // map kernel text executable and read-only.
    uint64 etext = (uint64)text_end;
    //_putint("etext: 0x", (uint64)etext, "\n");
    kvmmap(kpgtbl, PHY_BASE, PHY_BASE, etext - PHY_BASE, AP_RO_1, false, 0);
    kvmmap(kpgtbl, KERN_BASE + PHY_BASE, PHY_BASE, etext - PHY_BASE, AP_RO_1, false, 0);

    // map kernel data and the physical RAM we'll make use of.
    uint64 ekernel = (uint64)kernel_end;
    //_putint("ekernel: 0x", ekernel, "\n");
    kvmmap(kpgtbl, etext, etext, ekernel - etext, AP_RW_1, false, 0);
    kvmmap(kpgtbl, KERN_BASE + etext, etext, ekernel - etext, AP_RW_1, false, 0);

    _putstr("kvminit done!\n");

    // TODO: kernel stack
}

// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
// dst_level = 1: 2MB, dst_level = 0: 4KB
void kvmmap(ppte kpgtbl, uint64 vaddr, uint64 paddr, uint64 size, int perm, int dev_mem, int dst_level)
{
    if (mappages(kpgtbl, vaddr, size, paddr, perm, dev_mem, dst_level) != 0)
    {
        // TODO: panic
    }
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
// dst_level = 1: 2MB, dst_level = 0: 4KB
int mappages(ppte pagetable, uint64 vaddr, uint64 size, uint64 paddr, int perm, int dev_mem, int dst_level)
{
    uint64 a, last;
    ppte pte;
    int entry_attr = (!dst_level) ? ENTRY_PAGE : ENTRY_BLOCK;
    int step = PX_SIZE(dst_level);

    if (size == 0)
    {
        // TODO: panic
        return -1;
    }

    a = PX_ROUND_DOWN(vaddr, dst_level);
    last = PX_ROUND_DOWN(vaddr + size - 1, dst_level);
    for (;;)
    {
        //_putint("a: 0x", a, "\n");

        pte = walk(pagetable, a, 1, dst_level);
        //_putint("pte: 0x", (uint64)pte, "\n");

        if (pte == 0)
            return -1;
        if (*pte & (entry_attr | ENTRY_VALID))
        {
            _putstr("REMAPPING!\n");
            return -1;
            // TODO: panic remap
        }

        if (!dev_mem)
        {
            // normal memory
            *pte = paddr | ACCESS_FLAG | SH_IN_SH | perm | NON_SECURE_PA | MEM_ATTR_IDX_4 | entry_attr | ENTRY_VALID | UXN;
        }
        else
        {
            // device memory
            *pte = paddr | ACCESS_FLAG | perm | MEM_ATTR_IDX_0 | entry_attr | ENTRY_VALID;
        }

        //_putint("*pte: 0x", (uint64)(*pte), "\n");

        if (a == last)
        {
            break;
        }

        a += step;
        paddr += step;
    }
    return 0;
}

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// This is almost the same for armv8 4KB page tables.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.
// dst_level = 1: 2MB, dst_level = 0: 4KB
ppte walk(ppte pagetable, uint64 vaddr, int alloc, int dst_level)
{
    // TODO: check bound
    //_putint("vaddr: 0x", vaddr, "\n");

    for (int level = 2; level > dst_level; level--)
    {
        ppte pte = &pagetable[PX(level, vaddr)];
        //_putint("   level: 0x", level, "\n");
        //_putint("   pte: 0x", (uint64)pte, "\n");

        if (*pte & (ENTRY_TABLE | ENTRY_VALID))
        {
            pagetable = (ppte)(*pte & PX_ADDR_MASK);
            //_putint("   pagetable: 0x", (uint64)pagetable, "\n");
        }
        else
        {
            if (!alloc || (pagetable = (ppte)kalloc()) == 0)
                return 0;
            memset(pagetable, 0, PG_SIZE);
            //_putint("   level: 0x", (uint64)level, "\n");
            //_putint("   pagetable: 0x", (uint64)pagetable, "\n");
            *pte = ((uint64)pagetable & PX_ADDR_MASK) | (ENTRY_TABLE | ENTRY_VALID);
        }
    }
    return &pagetable[PX(dst_level, vaddr)];
}

// enable paging
void kvminithart(void)
{
    _putstr("kvminithart begin!\n");

    uint32 val32;
    uint64 val64;

    // flush TLB and cache
    _putstr("flushing tlb and instr cache\n");

    // flush Instr Cache
    asm("ic ialluis" : : : );

    // flush TLB
    asm("tlbi vmalle1" : : : );
    asm("dsb sy" : : : );

    // no trapping on FP/SIMD instructions
    val32 = 0x03 << 20;
    asm("msr cpacr_el1, %[v]" : : [v] "r" (val32) : );

    // monitor debug: all disabled
    asm("msr mdscr_el1, xzr" : : : );

    // set memory attribute indirection register
    // goto arm.h MEM_ATTR_IDX
    _putstr("setting memory attribute indirection register (mair_el1)\n");
    val64 = (uint64)0xff440c0400;
    asm("msr mair_el1, %[v]" : : [v] "r" (val64) : );
    asm("isb" : : : );

    // set translation control register
    _putstr("setting translation control register (tcr_el1)\n");
    val64 = (uint64)0x34b5203520;
    asm("msr tcr_el1, %[v]" : : [v] "r" (val64) : );
    asm("isb" : : : );

    // set translation table base register 1 (kernel)
    _putstr("setting translation table base register 1 (ttbr1_el1)\n");
    val64 = (uint64)kpgtbl;
    asm("msr ttbr1_el1, %[v]" : : [v] "r" (val64) : );

    // set translation table base register 0 (user)
    _putstr("setting translation table base register 0 (ttbr0_el1)\n");
    val64 = (uint64)kpgtbl;
    asm("msr ttbr0_el1, %[v]" : : [v] "r" (val64) : );
    asm("isb" : : : );

    // set system control register
    _putstr("setting system control register (sctlr_el1)\n");
    asm("mrs %[r], sctlr_el1" : [r] "=r" (val32) : : ); //0x0000000000c50838
    val32 = val32 | 0x01;
    asm("msr sctlr_el1, %[v]" : : [v] "r" (val32) : );
    asm("isb" : : : );

    // jump stack
    //mov     x0, sp
    //ldr     x1, =KERN_BASE
    //add     sp, x1, x0
    asm("mov %[r], sp" : [r] "=r" (val64) : : );
    asm("add sp, %[v1], %[v2]" : : [v1] "r" (val64), [v2] "r" (KERN_BASE) : );

    _putstr("kvminithart done!\n");
}