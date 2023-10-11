#include <stdio.h>

int main()
{
    for (int i = 0x40000000; i < 0x48000000; i += 0x200000)
    {
        printf("0x%x | PDE_PAGE,\n", i);
    }

    for (int i = 0x0; i < 0x0b000000; i += 0x200000)
    {
        if ((i >= 0x08000000 && i < 0x0b000000))
        {
            printf("0x%x | PDE_DEVICE,\n", i);
        }
        else printf("0x0,\n");
    }
}