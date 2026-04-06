#include "mini_uart.h"
#include "shell.h"
#include "dtb.h"
#include "exception.h"
#include "allocator.h"
#include "thread.h"
#include "utils.h"

extern void* _dtb_addr;

void set_current(thread_t* t) {
    asm volatile ("msr tpidr_el1, %0" :: "r"(t));
}

//thread test function
void foo() {
    thread_t* t = get_current();
    for (int i = 0; i < 10; i++) {
        mini_uart_send_string("Iteration: ");
        mini_uart_send_string(int2char(i));
        mini_uart_send_string("\r\nThread id:");
        mini_uart_send_string(int2char(t->id));
        mini_uart_send_string("\r\n");
        delay(1000000);
        schedule();
    }
    thread_exit();
}

void main()
{
    //initialization
    mini_uart_init();
    mini_uart_send_string("Hello from kernel program\r\n");
    mini_uart_send_string("DTB address: \r\n");
    mini_uart_send_hex((unsigned int)_dtb_addr);
    mini_uart_send_string("\r\n");
    fdt_traverse(_dtb_addr,get_ramfs_addr);
    irq_init();
    buddy_init();
    pool_init();


    //add a dummy thread to init tpidr_el1
    thread_t dummy;
    dummy.id = 0;
    dummy.state = THREAD_READY;
    dummy.stack = NULL;
    dummy.next = NULL;
    set_current(&dummy);

    // //create threads
    // for (int i = 0; i < 3; i++) {
    //     thread_create(foo);
    // }
    // idle();

    //shell function
    shell();
}