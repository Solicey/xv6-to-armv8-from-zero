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
    PROVIDE(edata = .);
    .bss : {
        *(.bss COMMON)
    }
    PROVIDE(ebss = .);

    PROVIDE(end = .);
}