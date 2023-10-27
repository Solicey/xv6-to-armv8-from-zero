// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"
#include "user.h"

char* argv[] = { "sh", 0 };

int main(void)
{
    int pid, wpid;

    if (open("console", O_RDWR) < 0)
    {
        mknod("console", CONSOLE, 0);
        open("console", O_RDWR);
    }

    dup(0);  // stdout
    printf("\n[USER] Hello World!\n\n");
    dup(0);  // stderr

    for (;;)
    {
        printf("init: starting sh\n");
        pid = fork();
        if (pid < 0)
        {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0)
        {
            //for (;;) { printf("C "); }
            exec("sh", argv);
            printf("init: exec sh failed\n");
            exit(1);
        }
        //for (;;) { printf("F "); }

        for (;;)
        {
            // this call to wait() returns if the shell exits,
            // or if a parentless process exits.
            wpid = wait(NULL);
            if (wpid == pid)
            {
                // the shell exited; restart it.
                break;
            }
            else if (wpid < 0)
            {
                printf("init: wait returned an error\n");
                exit(1);
            }
            else
            {
                // it was a parentless process; do nothing.
            }
        }
    }

    return 0;
}