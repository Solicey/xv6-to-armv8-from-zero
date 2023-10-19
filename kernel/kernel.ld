OUTPUT_ARCH(aarch64)

SECTIONS
{
    . = 0x40080000;

    .entry_s : {
        kernel/entry.o(.text .text.*)
        kernel/entry.o(.data .data.*)
        kernel/entry.o(.bss .bss.* COMMON)
        kernel/kstruct.o(.text .text.*)
        kernel/kstruct.o(.data .data.*)
        kernel/kstruct.o(.bss .bss.* COMMON)
    }

    . = ALIGN(0x1000);
    PROVIDE(eentry = .);
    . = . - eentry;
    .user_s : AT(eentry) {
        user/initcode(.text .text.*)
        user/initcode(.data .data.*)
        user/initcode(.bss .bss.* COMMON)
    }

    . = . + eentry;
    . = ALIGN(0x1000);
    PROVIDE(euser = .);
    . += 0xffff000000000000;

    .text : AT(euser) {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
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