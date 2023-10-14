#include "types.h"
#include "memlayout.h"
#include "defs.h"
#include "mmu.h"

void freerange(void* pa_start, void* pa_end);

extern char end[];

struct run
{
    struct run* next;
};

struct
{
    struct run* freelist;
} kmem;

void kinit()
{
    freerange(KERN_BASE + end, (void*)(KERN_BASE + PHY_STOP));
    cprintf("kinit done!\n");
}

void freerange(void* pa_start, void* pa_end)
{
    char* p;
    p = (char*)PG_ROUND_UP((uint64)pa_start);
    for (; p + PG_SIZE <= (char*)pa_end; p += PG_SIZE)
    {
        kfree(p);
    }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void* paddr)
{
    struct run* r;

    if (((uint64)paddr % PG_SIZE) != 0 || (char*)paddr < KERN_BASE + end || (uint64)paddr >= KERN_BASE + PHY_STOP)
        panic("kfree");

    // ?? fill with junks?
    //memset(paddr, 1, PG_SIZE);

    r = (struct run*)paddr;
    r->next = kmem.freelist;
    kmem.freelist = r;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void)
{
    struct run* r;

    r = kmem.freelist;
    if (r)
    {
        kmem.freelist = r->next;
        // ?? fill with junk?
    }

    return (void*)r;
}