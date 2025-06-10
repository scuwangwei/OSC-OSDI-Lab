.section ".text.boot"
.global _start

_start:
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, bss_init

hang:
    b       hang

bss_init:
    adr     x0, bss_begin
    adr     x1, bss_end
    sub     x2, x1, x0
    cbz     x2, set_stack_pointer

bss_loop:
    str     xzr, [x0], #8
    sub     x2, x2, #8
    cbnz     x2, bss_loop

set_stack_pointer:
    // from 0x400000 to 0x000000
    ldr     x0, =0x400000
    mov     sp, x0

    // go to main function
    bl      main

hang_after_main:
    b       hang_after_main