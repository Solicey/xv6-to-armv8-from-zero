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

    . = ALIGN(16);
    PROVIDE (data_end = .);

    .bss : {
        *(.bss .bss.* COMMON)
    }

    . = ALIGN(0x1000);
    PROVIDE (end = .);

    . = ALIGN(0x200000);
    PROVIDE (kernel_end = .);
}