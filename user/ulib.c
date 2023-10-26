#include "kernel/types.h"
#include "user.h"

//
// wrapper so that it's OK if main() does not call exit().
//
void _main()
{
    extern int main();
    main();
    exit(0);
}

char* strcpy(char* s, const char* t)
{
    char* os;

    os = s;
    while ((*s++ = *t++) != 0);
    return os;
}

int strcmp(const char* p, const char* q)
{
    while (*p && *p == *q)
        p++, q++;
    return (uchar)*p - (uchar)*q;
}

uint strlen(const char* s)
{
    int n;

    for (n = 0; s[n]; n++);
    return n;
}

void* memset(void* dst, int c, uint n)
{
    char* cdst = (char*)dst;
    int i;
    for (i = 0; i < n; i++)
    {
        cdst[i] = c;
    }
    return dst;
}

char* strchr(const char* s, char c)
{
    for (; *s; s++)
        if (*s == c)
            return (char*)s;
    return NULL;
}

char* gets(char* buf, int max)
{
    int i, cc;
    char c;

    for (i = 0; i + 1 < max; )
    {
        cc = read(0, &c, 1);
        if (cc < 1)
            break;
        buf[i++] = c;
        if (c == '\n' || c == '\r')
            break;
    }
    buf[i] = '\0';
    return buf;
}