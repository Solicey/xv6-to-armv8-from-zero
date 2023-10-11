#include "defs.h"

void start()
{
    uartinit();
    uartputc("Hello world\n");

    for (;;);
}