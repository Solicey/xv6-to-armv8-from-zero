#include "defs.h"
#include "types.h"
#include "mmu.h"
#include "memlayout.h"

/*
 * Given 'pgdir', a pointer to a page directory, walk returns
 * a pointer to the page table entry (PTE) for virtual address 'vaddr'.
 * This requires walking the four-level page table structure.
 *
 * The relevant page table page might not exist yet.
 * If this is true, and alloc == false, then walk returns NULL.
 * Otherwise, walk allocates a new page table page with kalloc.
 *   - If the allocation fails, walk returns NULL.
 *   - Otherwise, the new page is cleared, and walk returns
 *     a pointer into the new page table page.
*/
uint64* walk(uint64* pgdir, uint64 vaddr, int alloc)
{
    for (int level = 0; level < 3; level++)
    {
        uint64* pte = &pgdir[PX(level, vaddr)];
        if ((*pte & MM_TYPE_TABLE) == MM_TYPE_TABLE)
        {
            pgdir = (uint64*)P2V((*pte & PG_ADDR_MASK));
        }
        else if (*pte & MM_TYPE_VALID)
        {
            return NULL;
        }
        else
        {
            if (!alloc || (pgdir = (uint64*)kalloc()) == 0)
                return 0;
            memset(pgdir, 0, PG_SIZE);
            *pte = V2P((uint64)pgdir) | MM_TYPE_TABLE;
        }
    }

    return &pgdir[PX(3, vaddr)];
}

/*
 * Create PTEs for virtual addresses starting at vaddr that refer to
 * physical addresses starting at paddr. vaddr and size might **NOT**
 * be page-aligned.
 * Use permission bits perm|PTE_P|PTE_TABLE|PTE_AF for the entries.
 */
int mappages(uint64* pgdir, uint64 vaddr, uint64 paddr, uint64 size, int perm)
{
    uint64 a, last;
    uint64* pte;

    assert(size);

    a = PG_ROUND_DOWN(vaddr);
    last = PG_ROUND_DOWN(vaddr + size - 1);
    for (;;)
    {
        if ((pte = walk(pgdir, a, 1)) == 0)
        {
            cprintf("mappages: walk failed");
            return -1;
        }

        // remapping
        assert(!(*pte & MM_TYPE_VALID));

        *pte = (paddr & PG_ADDR_MASK) | perm | PTE_PAGE;

        if (a == last)
            break;

        a += PG_SIZE;
        paddr += PG_SIZE;
    }
    return 0;
}