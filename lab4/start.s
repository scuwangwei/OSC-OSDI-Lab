.section ".text.boot"
.global _start

//here use x1, do not use x0,x0 stores dtb addr,and just in case,hang other cpus except cpu0
_start:
    mrs     x1, mpidr_el1
    and     x1, x1, #0xFF
    cbz     x1, dtb_init

hang:
    b       hang

//get dtb address,in last stage,it pass dtb address to x0
dtb_init:
    ldr     x1, =_dtb_addr
    str     x0, [x1]

//initialize exception
exception_init:
    //switch from el2 to el1
    bl      from_el2_to_el1

    // setup interrupt vector base
    ldr x0, =exception_vector_table
    msr vbar_el1, x0

//init bss section
bss_init:
    adr     x0, bss_begin
    adr     x1, bss_end
    sub     x2, x1, x0
    cbz     x2, set_stack_pointer

bss_loop:
    str     xzr, [x0], #8
    sub     x2, x2, #8
    cbnz     x2, bss_loop

//put sp to correct address
set_stack_pointer:
    // from 0x400000 to 0x000000
    ldr     x0, =0x400000
    mov     sp, x0


jump_to_main:
    // go to main function
    bl      main



hang_after_main:
    b       hang_after_main

//define _dtb_addr in data section so that kernel program can access it
.section .data
.global _dtb_addr
_dtb_addr: .quad 0x0