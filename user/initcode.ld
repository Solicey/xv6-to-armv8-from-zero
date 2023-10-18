OUTPUT_ARCH(aarch64)
ENTRY(start)

SECTIONS
{
    . = 0x0;

    .text : {
        *(.text .text.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss COMMON)
    }

}