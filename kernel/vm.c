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
        //printf("level: %d\n", level);
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
            //printf("new pde: 0x%p\n", pde);
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
 */
int mappages(uint64* pde, uint64 vaddr, uint64 paddr, uint64 size, uint64 flags)
{
    uint64 a, last;
    uint64* pte;

    if (size == 0)
        panic("mappages: size");

    a = PG_ROUND_DOWN(vaddr);
    last = PG_ROUND_DOWN(vaddr + size - 1);
    for (;;)
    {
        //printf("pde: 0x%p, a: 0x%p\n", pde, a);
        if ((pte = walk(pde, a, 1)) == NULL)
        {
            printf("mappages: walk failed\n");
            return -1;
        }

        // remapping
        if (*pte & MM_TYPE_VALID)
            panic("mappages: remap");

        *pte = (paddr & PG_ADDR_MASK) | flags;

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

    if ((vaddr % PG_SIZE) != 0)
        panic("uvmunmap: not aligned");

    for (a = vaddr; a < vaddr + npages * PG_SIZE; a += PG_SIZE)
    {
        if ((pte = walk(pde, a, 0)) == NULL)
            panic("uvmunmap: walk");
        if ((*pte & MM_TYPE_VALID) == 0)
            panic("uvmunmap: not mapped");
        if ((*pte & MM_TYPE_MASK) != MM_TYPE_PAGE)
            panic("uvmunmap: not a leaf");

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
        else if (pte & MM_TYPE_VALID)
        {
            panic("freewalk: leaf");
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
// returns NULL if out of memory.
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

    if (size > PG_SIZE)
        panic("uvmfirst: more than a page");

    mem = kalloc();
    //printf("uvmfirst mem: 0x%p, src: 0x%p, size: %d\n", mem, src, size);
    memset(mem, 0, PG_SIZE);
    mappages(pde, 0, V2P(mem), PG_SIZE, USER_4K_PAGE_RO);
    memmove(mem, src, size);
}

// Switch to the user page table (TTBR0)
void uvmswitch(struct proc* p)
{
    uint64 val64;

    push_off();

    if (p->pagetable == NULL)
        panic("uvmswitch");

    val64 = (uint64)V2P(p->pagetable);
    //printf("uvmswitch pde: 0x%p\n", val64);
    lttbr0(val64);

    pop_off();
    //printf("uvmswitch done!\n");
}

// Look up a virtual address, return (KERNBASE + physical address),
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64 walkaddr(uint64* pde, uint64 vaddr)
{
    uint64* pte;
    uint64 paddr;

    if (vaddr >= MAXVA)
        return 0;

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
int copyout(uint64* pde, uint64 dstvaddr, char* src, uint64 len)
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

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint64 uvmdealloc(uint64* pde, uint64 oldsz, uint64 newsz)
{
    if (newsz >= oldsz)
        return oldsz;

    if (PG_ROUND_UP(newsz) < PG_ROUND_UP(oldsz))
    {
        int npages = (PG_ROUND_UP(oldsz) - PG_ROUND_UP(newsz)) / PG_SIZE;
        uvmunmap(pde, PG_ROUND_UP(newsz), npages, 1);
    }

    return newsz;
}

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64 uvmalloc(uint64* pde, uint64 oldsz, uint64 newsz, uint64 flags)
{
    char* mem;
    uint64 a;

    if (newsz < oldsz)
        return oldsz;

    oldsz = PG_ROUND_UP(oldsz);
    for (a = oldsz; a < newsz; a += PG_SIZE)
    {
        mem = kalloc();
        if (mem == NULL)
        {
            uvmdealloc(pde, a, oldsz);
            return 0;
        }
        memset(mem, 0, PG_SIZE);
        if (mappages(pde, a, (uint64)mem, PG_SIZE, flags) != 0)
        {
            kfree(mem);
            uvmdealloc(pde, a, oldsz);
            return 0;
        }
    }
    return newsz;
}

// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void uvmclear(uint64* pde, uint64 vaddr)
{
    uint64* pte;

    pte = walk(pde, vaddr, 0);
    if (pte == NULL)
        panic("uvmclear");

    *pte = ((*pte) & ~(MASK_4K_PAGE)) | KERNEL_4K_PAGE;
}

// Given a parent process's page table, copy
// its memory into a child's page table.
// Copies both the page table and the
// physical memory.
// returns 0 on success, -1 on failure.
// frees any allocated pages on failure.
int uvmcopy(uint64* old, uint64* new, uint64 size)
{
    uint64* pte;
    uint64 paddr, i;
    char* mem;

    for (i = 0; i < size; i += PG_SIZE)
    {
        if ((pte = walk(old, i, 0)) == NULL)
            panic("uvmcopy: pte should exist");
        if ((*pte & MM_TYPE_VALID) == 0)
            panic("uvmcopy: page not present");
        paddr = P2V(*pte & PG_ADDR_MASK);
        if ((mem = kalloc()) == NULL)
            goto err;
        memmove((void*)mem, (void*)paddr, PG_SIZE);
        if (mappages(new, i, (uint64)mem, PG_SIZE, (*pte & MASK_4K_PAGE)) != 0)
        {
            kfree(mem);
            goto err;
        }
    }
    return 0;

err:
    uvmunmap(new, 0, i / PG_SIZE, 1);
    return -1;
}