OUTPUT_ARCH(aarch64)
ENTRY(_entry)

SECTIONS
{
    . = 0x40080000;
    PROVIDE (text_start = .);

    .text : {
        *(.text .text.*)
        . = ALIGN(0x1000);
        PROVIDE (text_end = .);
    }
    
    .rodata : {
        . = ALIGN(16);
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }
    PROVIDE(edata = .);
    .bss : {
        *(.bss .bss.* COMMON)
    }
    PROVIDE(ebss = .);

    . = ALIGN(16);
    . = . + 0x1000;     /* 4kB stack memory */
    stack_top = .;

    PROVIDE(end = .);
}