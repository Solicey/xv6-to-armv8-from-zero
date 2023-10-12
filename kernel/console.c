#include <stdarg.h>
#include "types.h"
#include "defs.h"

int panicked;

void consoleinit(void)
{
    uartinit();
    cprintf("consoleinit done!\n");
}

static void printint(int64 x, int base, int sign)
{
    static char digit[] = "0123456789abcdef";
    static char buf[64];

    if (sign && x < 0)
    {
        x = -x;
        uartputc('-');
    }

    int i = 0;
    uint64 t = x;
    do
    {
        buf[i++] = digit[t % base];
    } while (t /= base);

    while (i--)
        uartputc(buf[i]);
}

void vsprintf(void (*putc)(char), const char *fmt, va_list ap)
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
void cprintf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsprintf(uartputc, fmt, ap);
    va_end(ap);
}

void panic(const char *fmt, ...)
{
    va_list ap;

    cprintf("\n!!!!!!!     panic      !!!!!!!\n");
    va_start(ap, fmt);
    vsprintf(uartputc, fmt, ap);
    va_end(ap);

    cprintf("\n%s:%d: kernel panic.\n", __FILE__, __LINE__);
    cprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    panicked = 1;
    for (;;);
}