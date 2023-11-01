#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

void primes(int x, int* a)
{
    if (x == 0)
        return;

    int fd[2];
    pipe(fd);
    int p = a[0];
    printf("prime %d\n", p);
    if (fork() == 0)
    {
        close(fd[0]);
        for (int i = 0; i < x; i++)
        {
            if (a[i] % p != 0)
                write(fd[1], &a[i], sizeof(int));
        }
        close(fd[1]);
        exit(0);
    }
    else
    {
        close(fd[1]);
        int t, cnt = 0;
        while (read(fd[0], &t, sizeof(int)) == sizeof(int))
        {
            a[cnt++] = t;
        }
        close(fd[0]);
        wait(0);
        primes(cnt, a);
    }

}

int main(int argc, char* argv[])
{
    int a[34];
    for (int i = 0; i < 34; i++)
        a[i] = i + 2;
    primes(34, a);
    exit(0);
}
