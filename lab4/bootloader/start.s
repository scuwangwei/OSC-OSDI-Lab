.section ".text.relo"
.global _start

// relocation,in this program only cpu0 on
_start:

    ldr     x10, =bl_ori_addr //x10 = bootloader current start address(0x80000)
    ldr     x11, =_blsize //x11 = bootloader size
    add     x11, x11, x10 //x11 = current end address of bootloader
    ldr     x12, =_stext //x12 = start address of bootloader after relocation

1:
    cmp     x10, x11 //compare x10 with x11,if equal then end of relocation,traverse bootloader memory area from start to end
    b.eq    2f //if done go to 2
    //copy data
    ldr     x13, [x10] //move x10 to x13
    str     x13, [x12] //move x13 to x12
    //move to next doubleword
    add     x10, x10, #8
    add     x12, x12, #8
    b       1b //go to 1(loop)

//jump to the address where bootloader relocated
2:
    ldr     x14, =_bl_enrty
    br      x14

.section ".text.boot"
.global _bl_enrty

//cpu0 execute initialization
_bl_enrty:
    mrs     x20, mpidr_el1
    and     x20, x20, #0xFF
    cbz     x20, dtb_init

hang:
    b       hang

//record dtb address,and pass it to kernel main in bootloader_main function(IMPORTAN:it must be assigned address value after relocation,otherwise it will have unexpected action)
dtb_init:
    ldr     x1, =_dtb_addr
    str     x0, [x1]

//bss init
bss_init:
    adr     x20, bss_begin
    adr     x21, bss_end
    sub     x22, x21, x20
    cbz     x22, set_stack_pointer

bss_loop:
    str     xzr, [x20], #8
    sub     x22, x22, #8
    cbnz     x22, bss_loop

//put sp to correct address
set_stack_pointer:
    // from 0x400000 to 0x000000
    ldr     x20, =0x400000
    mov     sp, x20

    // go to bootloader_main function
    bl      bootloader_main

hang_after_main:
    b       hang_after_main

//define _dtb_addr in data section so that bootloader program can access it
.section .data
.global _dtb_addr
_dtb_addr: .quad 0x0