#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "arm.h"
#include "defs.h"

// first address after kernel.
// defined by kernel.ld.
extern char end[];

struct run
{
    struct run* next;
};

struct
{
    // TODO: lock
    struct run* freelist;
} kmem;

void freerange(void* pa_start, void* pa_end);

void kinit()
{
    // TODO: lock
    freerange((void*)end, (void*)PHY_STOP);
    _putstr("kinit done!\n");
}

void freerange(void* pa_start, void* pa_end)
{
    char* p;
    //p = (char*)PG_ROUND_UP((uint64)pa_start);
    p = (char*)PG_ROUND_DOWN((uint64)pa_end) - PG_SIZE;
    for (; p >= (char*)pa_start; p -= PG_SIZE)
    {
        kfree(p);
    }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void* pa)
{
    struct run* r;

    if (((uint64)pa % PG_SIZE) != 0 || (char*)pa < end || (uint64)pa >= PHY_STOP)
    {
        // TODO: panic
        return;
    }

    // fill
    //memset(pa, 1, PG_SIZE);

    r = (struct run*)pa;

    // TODO: get lock
    r->next = kmem.freelist;
    kmem.freelist = r;
    // TODO: free lock
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void)
{
    struct run *r;

    // TODO: get lock
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    // TODO: free lock

    //_putint("r: 0x", (uint64)r, "\n");

    //if (r)
        //memset((char*)r, 5, PG_SIZE);   // fill with junk
    return (void*)r;
}