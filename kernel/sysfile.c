#include "types.h"
#include "defs.h"
#include "proc.h"
#include "param.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int argfd(int n, int* pfd, struct file** pf)
{
    int fd;
    struct file* f;

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
    char path[MAXPATH], * argv[MAXARG];
    int i;
    uint64 uargv, uarg;

    argaddr(2, &uargv);
    //printf("exec uargv: 0x%p\n", uargv);
    if (argstr(1, path, MAXPATH) < 0)
        return -1;
    //printf("exec path: %s\n", path);

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
        //printf("   exec argv[%d]: %s\n", i, argv[i]);
    }

    int ret = exec(path, argv);

    //printf("exec get ret!\n");

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
    struct file* f;
    int n;
    uint64 p;

    argaddr(2, &p);
    argint(3, &n);

    if (argfd(1, NULL, &f) < 0)
        return -1;

    return filewrite(f, p, n);
}

uint64 sys_read(void)
{
    struct file* f;
    int n;
    uint64 p;

    argaddr(2, &p);
    argint(3, &n);

    if (argfd(1, NULL, &f) < 0)
        return -1;

    return fileread(f, p, n);
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
static struct inode* create(char* path, short type, short major, short minor)
{
    struct inode* ip, * dp;
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
    struct inode* ip;
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
    struct file* f;
    int fd;

    if (argfd(1, NULL, &f) < 0)
        return -1;
    if ((fd = fdalloc(f)) < 0)
        return -1;
    filedup(f);
    return fd;
}

uint64 sys_chdir(void)
{
    char path[MAXPATH];
    struct inode* ip;
    struct proc* p = myproc();

    begin_op();
    if (argstr(1, path, MAXPATH) < 0 || (ip = namei(path)) == 0)
    {
        end_op();
        return -1;
    }
    ilock(ip);
    if (ip->type != T_DIR)
    {
        iunlockput(ip);
        end_op();
        return -1;
    }
    iunlock(ip);
    iput(p->cwd);
    end_op();
    p->cwd = ip;
    return 0;
}

uint64 sys_close(void)
{
    int fd;
    struct file* f;

    if (argfd(1, &fd, &f) < 0)
        return -1;
    myproc()->ofile[fd] = NULL;
    fileclose(f);
    return 0;
}

uint64 sys_fstat(void)
{
    struct file* f;
    uint64 st; // user pointer to struct stat

    argaddr(2, &st);
    if (argfd(1, 0, &f) < 0)
        return -1;
    return filestat(f, st);
}

uint64 sys_mkdir(void)
{
    char path[MAXPATH];
    struct inode* ip;

    begin_op();
    if (argstr(1, path, MAXPATH) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0)
    {
        end_op();
        return -1;
    }
    iunlockput(ip);
    end_op();
    return 0;
}

// Is the directory dp empty except for "." and ".." ?
static int isdirempty(struct inode* dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de))
    {
        if (readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
            panic("isdirempty: readi");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

uint64 sys_unlink(void)
{
    struct inode* ip, * dp;
    struct dirent de;
    char name[DIRSIZ], path[MAXPATH];
    uint off;

    if (argstr(1, path, MAXPATH) < 0)
        return -1;

    begin_op();
    if ((dp = nameiparent(path, name)) == NULL)
    {
        end_op();
        return -1;
    }

    ilock(dp);

    // Cannot unlink "." or "..".
    if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
        goto bad;

    if ((ip = dirlookup(dp, name, &off)) == NULL)
        goto bad;
    ilock(ip);

    if (ip->nlink < 1)
        panic("unlink: nlink < 1");
    if (ip->type == T_DIR && !isdirempty(ip))
    {
        iunlockput(ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));
    if (writei(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
        panic("unlink: writei");
    if (ip->type == T_DIR)
    {
        dp->nlink--;
        iupdate(dp);
    }
    iunlockput(dp);

    ip->nlink--;
    iupdate(ip);
    iunlockput(ip);

    end_op();

    return 0;

bad:
    iunlockput(dp);
    end_op();
    return -1;
}

uint64 sys_pipe(void)
{
    uint64 fdarray; // user pointer to array of two integers
    struct file* rf, * wf;
    int fd0, fd1;
    struct proc* p = myproc();

    argaddr(1, &fdarray);
    if (pipealloc(&rf, &wf) < 0)
        return -1;
    fd0 = -1;
    if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0)
    {
        if (fd0 >= 0)
            p->ofile[fd0] = NULL;
        fileclose(rf);
        fileclose(wf);
        return -1;
    }
    if (copyout(p->pagetable, fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
        copyout(p->pagetable, fdarray + sizeof(fd0), (char*)&fd1, sizeof(fd1)) < 0)
    {
        p->ofile[fd0] = NULL;
        p->ofile[fd1] = NULL;
        fileclose(rf);
        fileclose(wf);
        return -1;
    }
    return 0;
}

// Create the path new as a link to the same inode as old.
uint64 sys_link(void)
{
    char name[DIRSIZ], new[MAXPATH], old[MAXPATH];
    struct inode* dp, * ip;

    if (argstr(1, old, MAXPATH) < 0 || argstr(2, new, MAXPATH) < 0)
        return -1;

    begin_op();
    if ((ip = namei(old)) == NULL)
    {
        end_op();
        return -1;
    }

    ilock(ip);
    if (ip->type == T_DIR)
    {
        iunlockput(ip);
        end_op();
        return -1;
    }

    ip->nlink++;
    iupdate(ip);
    iunlock(ip);

    if ((dp = nameiparent(new, name)) == NULL)
        goto bad;
    ilock(dp);
    if (dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0)
    {
        iunlockput(dp);
        goto bad;
    }
    iunlockput(dp);
    iput(ip);

    end_op();

    return 0;

bad:
    ilock(ip);
    ip->nlink--;
    iupdate(ip);
    iunlockput(ip);
    end_op();
    return -1;
}