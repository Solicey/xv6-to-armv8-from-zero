#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"
#include "param.h"
#include "elf.h"
#include "file.h"

// Load a program segment into pagetable at virtual address vaddr.
// vaddr must be page-aligned
// and the pages from vaddr to vaddr+size must already be mapped.
// Returns 0 on success, -1 on failure.
static int loadseg(uint64* pde, uint64 vaddr, struct inode* ip, uint offset, uint size)
{
    uint i, n;
    uint64 paddr;

    for (i = 0; i < size; i += PG_SIZE)
    {
        paddr = walkaddr(pde, vaddr + i);
        if (paddr == 0)
            panic("loadseg: address should exist");
        if (size - i < PG_SIZE)
            n = size - i;
        else
            n = PG_SIZE;
        if (readi(ip, 0, (uint64)paddr, offset + i, n) != n)
            return -1;
    }

    return 0;
}

uint64 getflags(int flags)
{
    if ((flags & 0x1) && (flags & 0x2))
        return USER_4K_PAGE_RW;
    if (flags & 0x1)
        return USER_4K_PAGE_RO;
    if (flags & 0x2)
        return USER_4K_PAGE_RW_XN;
    return USER_4K_PAGE_RO_XN;
}

int exec(char* path, char** argv)
{
    char* s, * last;
    int i, off;
    uint64 argc, sz = 0, sp, ustack[MAXARG], stackbase;
    struct elfhdr elf;
    struct inode* ip;
    struct proghdr ph;
    uint64* pde = NULL, * oldpde;
    struct proc* p = myproc();

    begin_op();

    if ((ip = namei(path)) == NULL)
    {
        end_op();
        return -1;
    }

    //printf("exec path: %s\n", path);

    ilock(ip);

    // Check ELF header
    if (readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf))
        goto bad;

    if (elf.magic != ELF_MAGIC)
        goto bad;

    if ((pde = uvmcreate()) == NULL)
        goto bad;

    // Load program into memory.
    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph))
    {
        if (readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
            goto bad;
        if (ph.type != ELF_PROG_LOAD)
            continue;
        if (ph.memsz < ph.filesz)
            goto bad;
        if (ph.vaddr % PG_SIZE != 0)
            goto bad;
        uint64 sz1;
        if ((sz1 = uvmalloc(pde, sz, ph.vaddr + ph.memsz, getflags(ph.flags))) == 0)
            goto bad;
        sz = sz1;
        if (loadseg(pde, ph.vaddr, ip, ph.off, ph.filesz) < 0)
            goto bad;
    }
    iunlockput(ip);
    end_op();
    ip = NULL;

    p = myproc();
    uint64 oldsz = p->size;

    // Allocate two pages at the next page boundary.
    // Make the first inaccessible as a stack guard.
    // Use the second as the user stack.
    sz = PG_ROUND_UP(sz);
    uint64 sz1;
    if ((sz1 = uvmalloc(pde, sz, sz + 2 * PG_SIZE, USER_4K_PAGE_RW_XN)) == 0)
        goto bad;
    sz = sz1;
    uvmclear(pde, sz - 2 * PG_SIZE);
    sp = sz;
    stackbase = sp - PG_SIZE;

    // Push argument strings, prepare rest of stack in ustack.
    for (argc = 0; argv[argc]; argc++)
    {
        if (argc >= MAXARG)
            goto bad;
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16; // riscv sp must be 16-byte aligned
        if (sp < stackbase)
            goto bad;
        if (copyout(pde, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
            goto bad;
        ustack[argc] = sp;
    }
    ustack[argc] = 0;

    // push the array of argv[] pointers.
    sp -= (argc + 1) * sizeof(uint64);
    sp -= sp % 16;
    if (sp < stackbase)
        goto bad;
    if (copyout(pde, sp, (char*)ustack, (argc + 1) * sizeof(uint64)) < 0)
        goto bad;

    // arguments to user main(argc, argv)
    // argc is returned via the system call return
    // value, which goes in x0.
    p->trapframe->x1 = sp;

    // Save program name for debugging.
    for (last = s = path; *s; s++)
        if (*s == '/')
            last = s + 1;
    safestrcpy(p->name, last, sizeof(p->name));

    // Commit to the user image.
    oldpde = p->pagetable;
    p->pagetable = pde;
    p->size = sz;
    p->trapframe->pc = elf.entry;   // initial program counter = main
    p->trapframe->sp = sp;          // initial stack pointer

    uvmswitch(p);
    uvmfree(oldpde, oldsz);

    return argc;    // this ends up in x0, the first argument to main(argc, argv)

bad:
    if (pde)
        uvmfree(pde, sz);
    if (ip)
    {
        iunlockput(ip);
        end_op();
    }
    return -1;
}