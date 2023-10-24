#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"

// lock to avoid interleaving concurrent printf's.
static struct
{
    struct spinlock lock;
    int locking;
} pr;

volatile int assertion_failed = -1;

static void printint(int64 x, int base, int sign)
{
    static char digit[] = "0123456789abcdef";
    static char buf[64];

    if (sign && x < 0)
    {
        x = -x;
        consputc('-');
    }

    int i = 0;
    uint64 t = x;
    do
    {
        buf[i++] = digit[t % base];
    } while (t /= base);

    while (i--)
        consputc(buf[i]);
}

static void vprintf(void (*putc)(char), const char *fmt, va_list ap)
{
    int i, c;
    char *s;
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
    {
        if (c != '%')
        {
            putc(c);
            continue;
        }

        int l = 0;
        for (; fmt[i + 1] == 'l'; i++)
            l++;

        if (!(c = fmt[++i] & 0xff))
            break;

        switch (c)
        {
        case 'u':
            if (l == 2) printint(va_arg(ap, int64), 10, 0);
            else printint(va_arg(ap, int), 10, 0);
            break;
        case 'd':
            if (l == 2) printint(va_arg(ap, int64), 10, 1);
            else printint(va_arg(ap, int), 10, 1);
            break;
        case 'x':
            if (l == 2) printint(va_arg(ap, int64), 16, 0);
            else printint(va_arg(ap, int), 16, 0);
            break;
        case 'p':
            consputc('0');
            consputc('x');
            printint((int64)va_arg(ap, void *), 16, 0);
            break;
        case 'c':
            putc(va_arg(ap, int));
            break;
        case 's':
            if ((s = (char*)va_arg(ap, char*)) == 0)
                s = "(null)";
            for (; *s; s++)
                putc(*s);
            break;
        case '%':
            putc('%');
            break;
        default:
            putc('%');
            putc(c);
            break;
        }
    }
}

/* Print to the console. */
void printf(const char *fmt, ...)
{
    va_list ap;

    int locking = pr.locking;
    if (locking)
        acquire(&pr.lock);

    va_start(ap, fmt);
    vprintf(consputc, fmt, ap);
    va_end(ap);

    if (locking)
        release(&pr.lock);
}

void check_assertion(void)
{
    pr.locking = 0;
    if (assertion_failed < 0)
        assertion_failed = cpuid();
    else
        for (;;);
}

void printfinit(void)
{
    initlock(&pr.lock, "pr");
    pr.locking = 1;
}