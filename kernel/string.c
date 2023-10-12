#include "types.h"

void* memset(void* dst, int c, uint n)
{
    char* cdst = (char*)dst;
    for (int i = 0; i < n; i++)
    {
        cdst[i] = c;
    }
    return dst;
}

int memcmp(const void* v1, const void* v2, uint n)
{
    const uchar *s1, *s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0)
    {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void* memmove(void* dst, const void* src, uint n)
{
    const char* s;
    char* d;

    if (n == 0)
        return dst;

    s = src;
    d = dst;
    if (s < d && s + n > d)
    {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    }
    else
    {
        while (n-- > 0)
            *d++ = *s++;
    }

    return dst;
}

void* memcpy(void* dst, const void* src, uint n)
{
    return memmove(dst, src, n);
}

int strlen(const char* s)
{
    int n;
    for (n = 0; s[n]; n++);
    return n;
}