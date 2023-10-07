OUTPUT_ARCH(aarch64)
ENTRY(_entry)

SECTIONS
{
    . = 0x40080000;

    .text : {
        *(.text .text.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss COMMON)
    }

    . = ALIGN(16);
    . = . + 0x1000;     /* 4kB stack memory */
    stack_top = .;
}