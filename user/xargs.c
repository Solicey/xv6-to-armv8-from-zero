#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user.h"

int main(int argc, char* argv[])
{
    char buf[64], str[64];
    char* nargv[MAXARG];
    int r, nargc = argc, p = 0, lp = -1;
    for (int i = 0; i < nargc; i++)
    {
        nargv[i] = argv[i];
    }
    while ((r = read(0, buf, sizeof(buf))) > 0)
    {
        for (int i = 0; i < r; i++)
        {
            if (buf[i] == '\n')
            {
                if (lp != -1)
                {
                    str[p++] = '\0';
                    nargv[nargc++] = &str[lp];
                    lp = -1;
                }
                nargv[nargc] = 0;
                if (fork() == 0)
                    exec(nargv[1], &nargv[1]);
                wait(0);
                p = 0;
                lp = -1;
                nargc = argc;
            }
            else if (buf[i] == ' ')
            {
                if (lp != -1)
                {
                    str[p++] = '\0';
                    nargv[nargc++] = &str[lp];
                    lp = -1;
                }
            }
            else
            {
                if (lp == -1)
                    lp = p;
                str[p++] = buf[i];
            }
        }
    }

    exit(0);
}
