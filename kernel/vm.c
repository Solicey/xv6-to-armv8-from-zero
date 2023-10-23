#include "defs.h"
#include "types.h"
#include "mmu.h"
#include "memlayout.h"

/*
 * Given 'pde', a pointer to a page directory, walk returns
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
uint64* walk(uint64* pde, uint64 vaddr, int alloc)
{
    for (int level = 0; level < 3; level++)
    {
        //cprintf("level: %d\n", level);
        uint64* pte = &pde[PX(level, vaddr)];
        if ((*pte & MM_TYPE_MASK) == MM_TYPE_TABLE)
        {
            pde = (uint64*)P2V((*pte & PG_ADDR_MASK));
        }
        else if (*pte & MM_TYPE_VALID)
        {
            return NULL;
        }
        else
        {
            if (!alloc || (pde = (uint64*)kalloc()) == 0)
                return 0;
            //cprintf("new pde: 0x%p\n", pde);
            memset(pde, 0, PG_SIZE);
            *pte = V2P((uint64)pde) | MM_TYPE_TABLE;
        }
    }

    return &pde[PX(3, vaddr)];
}

/*
 * Create PTEs for virtual addresses starting at vaddr that refer to
 * physical addresses starting at paddr. vaddr and size might **NOT**
 * be page-aligned.
 * Use permission bits perm|PTE_P|PTE_TABLE|PTE_AF for the entries.
 */
int mappages(uint64* pde, uint64 vaddr, uint64 paddr, uint64 size, int perm)
{
    uint64 a, last;
    uint64* pte;

    assert(size);

    a = PG_ROUND_DOWN(vaddr);
    last = PG_ROUND_DOWN(vaddr + size - 1);
    for (;;)
    {
        //cprintf("pde: 0x%p, a: 0x%p\n", pde, a);
        if ((pte = walk(pde, a, 1)) == NULL)
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

// Remove npages of mappings starting from va. va must be
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
// Can only free 4K pages
void uvmunmap(uint64* pde, uint64 vaddr, uint64 npages, int do_free)
{
    uint64 a;
    uint64* pte;

    assert(vaddr % PG_SIZE == 0);

    for (a = vaddr; a < vaddr + npages * PG_SIZE; a += PG_SIZE)
    {
        assert(pte = walk(pde, a, 0));
        assert(*pte & MM_TYPE_VALID);
        assert((*pte & MM_TYPE_MASK) == MM_TYPE_PAGE);

        if (do_free)
        {
            uint64 paddr = P2V((*pte & PG_ADDR_MASK));
            kfree((void*)paddr);
        }

        *pte = 0;
    }
}

// Recursively free page-table pages.
// All leaf mappings must already have been removed.
void freewalk(uint64* pde)
{
    for (int i = 0; i < PX_ENTRY_CNT; i++)
    {
        uint64 pte = pde[i];
        if ((pte & MM_TYPE_MASK) == MM_TYPE_TABLE)
        {
            uint64* child = (uint64*)P2V((pte & PG_ADDR_MASK));
            freewalk(child);
            pde[i] = 0;
        }
        else
        {
            assert(!(pte & MM_TYPE_VALID));
        }
    }
    kfree((void*)pde);
}

// Free user memory pages,
// then free page-table pages.
void uvmfree(uint64* pde, uint64 size)
{
    if (size > 0)
        uvmunmap(pde, 0, PG_ROUND_UP(size) / PG_SIZE, 1);
    freewalk(pde);
}

// create an empty user page table.
// returns 0 if out of memory.
uint64* uvmcreate(void)
{
    uint64* pde;
    pde = (uint64*)kalloc();
    if (pde == NULL)
        return NULL;
    memset(pde, 0, PG_SIZE);
    return pde;
}

// Load the user initcode into address 0 of pagetable,
// for the very first process.
// size must be less than a page.
void uvmfirst(uint64* pde, char* src, uint size)
{
    char* mem;

    assert(size <= PG_SIZE);

    mem = kalloc();
    cprintf("uvmfirst mem: 0x%p, src: 0x%p, size: %d\n", mem, src, size);
    memset(mem, 0, PG_SIZE);
    mappages(pde, 0, V2P(mem), PG_SIZE, AP_RW | AP_USER);
    memmove(mem, src, size);
}

// Switch to the user page table (TTBR0)
void uvmswitch(struct proc* p)
{
    uint64 val64;

    push_off();

    assert(p->pagetable);
    val64 = (uint64)V2P(p->pagetable);
    //cprintf("uvmswitch pde: 0x%p\n", val64);
    lttbr0(val64);

    pop_off();
    cprintf("uvmswitch done!\n");
}

// Look up a virtual address, return (KERNBASE + physical address),
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64 walkaddr(uint64* pde, uint64 vaddr)
{
    uint64* pte;
    uint64 paddr;

    assert(!(vaddr & KERN_BASE));

    pte = walk(pde, vaddr, 0);
    if (pte == NULL)
        return 0;
    if ((*pte & MM_TYPE_MASK) != MM_TYPE_PAGE)
        return 0;
    if ((*pte & AP_USER) == 0)
        return 0;
    paddr = P2V((*pte) & PG_ADDR_MASK);
    return paddr;
}

// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcvaddr in a given page table,
// until a '\0', or max.
// Return 0 on success, -1 on error.
int copyinstr(uint64* pde, char* dst, uint64 srcvaddr, uint64 max)
{
    uint64 n, va0, pa0;
    int got_null = 0;

    while (got_null == 0 && max > 0)
    {
        va0 = PG_ROUND_DOWN(srcvaddr);
        pa0 = walkaddr(pde, va0);
        if (pa0 == NULL)
            return -1;
        n = PG_SIZE - (srcvaddr - va0);
        if (n > max)
            n = max;

        char* p = (char*)(pa0 + (srcvaddr - va0));
        while (n > 0)
        {
            if (*p == '\0')
            {
                *dst = '\0';
                got_null = 1;
                break;
            }
            else
            {
                *dst = *p;
            }
            n--;
            max--;
            p++;
            dst++;
        }

        srcvaddr = va0 + PG_SIZE;
    }

    if (got_null)
        return 0;
    else
        return -1;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcvaddr in a given page table.
// Return 0 on success, -1 on error.
int copyin(uint64* pde, char* dst, uint64 srcvaddr, uint64 len)
{
    uint64 n, va0, pa0;
    while (len > 0)
    {
        va0 = PG_ROUND_DOWN(srcvaddr);
        pa0 = walkaddr(pde, va0);
        if (pa0 == NULL)
            return -1;
        n = PG_SIZE - (srcvaddr - va0);
        if (n > len)
            n = len;
        memmove(dst, (void*)(pa0 + (srcvaddr - va0)), n);

        len -= n;
        dst += n;
        srcvaddr = va0 + PG_SIZE;
    }
    return 0;
}

// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int copyout(uint64* pde, uint64 dstvaddr, char *src, uint64 len)
{
    uint64 n, va0, pa0;
    while (len > 0)
    {
        va0 = PG_ROUND_DOWN(dstvaddr);
        pa0 = walkaddr(pde, va0);
        if (pa0 == 0)
            return -1;
        n = PG_SIZE - (dstvaddr - va0);
        if (n > len)
            n = len;
        memmove((void*)(pa0 + (dstvaddr - va0)), src, n);

        len -= n;
        src += n;
        dstvaddr = va0 + PG_SIZE;
    }
    return 0;
}