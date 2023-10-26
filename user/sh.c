#include "kernel/types.h"

#include "user.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

struct cmd
{
    int type;
};

struct execcmd
{
    int type;
    char* argv[MAXARGS];
    char* eargv[MAXARGS];
};

int getcmd(char* buf, int nbuf)
{
    write(2, "$ ", 2);
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

int main(void)
{
    return 0;
}