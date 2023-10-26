#include "types.h"
#include "defs.h"
#include "proc.h"
#include "param.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int argfd(int n, int *pfd, struct file **pf)
{
    int fd;
    struct file *f;

    argint(n, &fd);
    if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == NULL)
        return -1;
    if (pfd)
        *pfd = fd;
    if (pf)
        *pf = f;
    return 0;
}

uint64 sys_exec(void)
{
    char path[MAXPATH], *argv[MAXARG];
    int i;
    uint64 uargv, uarg;

    argaddr(2, &uargv);
    printf("exec uargv: 0x%p\n", uargv);
    if (argstr(1, path, MAXPATH) < 0)
        return -1;
    printf("exec path: %s\n", path);

    memset(argv, 0, sizeof(argv));
    for (i = 0; ; i++)
    {
        if (i >= NELEM(argv))
            goto bad;
        if (fetchaddr(uargv + sizeof(uint64) * i, (uint64*)&uarg) < 0)
            goto bad;
        if (uarg == 0)
        {
            argv[i] = 0;
            break;
        }
        argv[i] = kalloc();
        if (argv[i] == NULL)
            goto bad;
        if (fetchstr(uarg, argv[i], PG_SIZE) < 0)
            goto bad;
        printf("   exec argv[%d]: %s\n", i, argv[i]);
    }

    int ret = exec(path, argv);

    printf("exec get ret!\n");

    for (i = 0; i < NELEM(argv) && argv[i] != 0; i++)
        kfree(argv[i]);

    return ret;

bad:
    for (i = 0; i < NELEM(argv) && argv[i] != 0; i++)
        kfree(argv[i]);
    return -1;
}

uint64 sys_write(void)
{
    struct file *f;
    int n;
    uint64 p;

    argaddr(2, &p);
    argint(3, &n);

    if (argfd(1, NULL, &f) < 0)
        return -1;

    return filewrite(f, p, n);
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int fdalloc(struct file* f)
{
    int fd;
    struct proc* p = myproc();

    for (fd = 0; fd < NOFILE; fd++)
    {
        if (p->ofile[fd] == NULL)
        {
            p->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

// If success, return ip with lock
static struct inode* create(char *path, short type, short major, short minor)
{
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if ((dp = nameiparent(path, name)) == NULL)
        return NULL;

    ilock(dp);

    if ((ip = dirlookup(dp, name, 0)) != NULL)
    {
        iunlockput(dp);
        ilock(ip);
        if (type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
            return ip;
        iunlockput(ip);
        return 0;
    }

    if ((ip = ialloc(dp->dev, type)) == NULL)
    {
        iunlockput(dp);
        return 0;
    }

    ilock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);

    if (type == T_DIR)
    {
        // Create . and .. entries.
        // No ip->nlink++ for ".": avoid cyclic ref count.
        if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
            goto fail;
    }

    if (dirlink(dp, name, ip->inum) < 0)
        goto fail;

    if (type == T_DIR)
    {
        // now that success is guaranteed:
        dp->nlink++;  // for ".."
        iupdate(dp);
    }

    iunlockput(dp);

    return ip;

fail:
    // something went wrong. de-allocate ip.
    ip->nlink = 0;
    iupdate(ip);
    iunlockput(ip);
    iunlockput(dp);
    return 0;
}

uint64 sys_open(void)
{
    char path[MAXPATH];
    int fd, omode;
    struct file* f;
    struct inode* ip;
    int n;

    argint(2, &omode);
    if ((n = argstr(1, path, MAXPATH)) < 0)
        return -1;

    begin_op();

    if (omode & O_CREATE)
    {
        ip = create(path, T_FILE, 0, 0);
        if (ip == NULL)
        {
            end_op();
            return -1;
        }
    }
    else
    {
        if ((ip = namei(path)) == NULL)
        {
            end_op();
            return -1;
        }
        ilock(ip);
        if (ip->type == T_DIR && omode != O_RDONLY)
        {
            iunlockput(ip);
            end_op();
            return -1;
        }
    }

    if (ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV))
    {
        iunlockput(ip);
        end_op();
        return -1;
    }

    if ((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0)
    {
        if (f)
            fileclose(f);
        iunlockput(ip);
        end_op();
        return -1;
    }

    if (ip->type == T_DEVICE)
    {
        f->type = FD_DEVICE;
        f->major = ip->major;
    }
    else
    {
        f->type = FD_INODE;
        f->off = 0;
    }
    f->ip = ip;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    if ((omode & O_TRUNC) && ip->type == T_FILE)
    {
        itrunc(ip);
    }

    iunlock(ip);
    end_op();

    return fd;
}

uint64 sys_mknod(void)
{
    struct inode *ip;
    char path[MAXPATH];
    int major, minor;

    begin_op();

    argint(2, &major);
    argint(3, &minor);
    if ((argstr(1, path, MAXPATH)) < 0 || (ip = create(path, T_DEVICE, major, minor)) == NULL)
    {
        end_op();
        return -1;
    }
    iunlockput(ip);

    end_op();
    return 0;
}

uint64 sys_dup(void)
{
    struct file *f;
    int fd;

    if (argfd(1, NULL, &f) < 0)
        return -1;
    if ((fd = fdalloc(f)) < 0)
        return -1;
    filedup(f);
    return fd;
}