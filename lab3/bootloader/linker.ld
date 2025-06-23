SECTIONS
{
    . = 0x60000;

    _stext = .;

    bl_ori_addr = 0x80000;
    .text : {
        *(.text.relo)
        _bl_enrty = .;
        *(.text.boot)
        *(.text)
        *(.rodata)
    }

    .data : { *(.data) }

    . = ALIGN(0x8);

    bss_begin = .;

    .bss : { *(.bss*) }

    bss_end = .;

    _blsize = bss_end - _stext;
}
