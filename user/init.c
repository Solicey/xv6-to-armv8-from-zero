// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"
#include "user.h"

int main(void)
{
    if (open("console", O_RDWR) < 0)
    {
        mknod("console", CONSOLE, 0);
        open("console", O_RDWR);
    }

    dup(0);  // stdout
    printf("\n[USER] Hello World!\n\n");
    dup(0);  // stderr

    return 0;
}