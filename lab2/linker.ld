SECTIONS
{
    . = 0x80000;

    .text.boot : { *(.text.boot) }

    .text : { *(.text) }

    .rodata : { *(.rodata) }

    .data : { *(.data) }

    . = ALIGN(0x8);

    bss_begin = .;

    .bss : { *(.bss*) }

    bss_end = .;

    PROVIDE(heap_start = .);

    .head (NOLOAD) : {
        *(.heap)
        . = . + 0x10000;
    }

    PROVIDE(heap_end = .);
}