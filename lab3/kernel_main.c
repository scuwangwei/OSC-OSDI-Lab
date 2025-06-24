#include "mini_uart.h"
#include "shell.h"
#include "dtb.h"
#include "exception.h"

extern void* _dtb_addr;
void main()
{
     // initialize mini uart serial port
    mini_uart_init();
    mini_uart_send_string("Hello from kernel program\r\n");
    mini_uart_send_string("DTB address: \r\n");
    mini_uart_send_hex((unsigned int)_dtb_addr);
    mini_uart_send_string("\r\n");
    fdt_traverse(_dtb_addr,get_ramfs_addr);
    irq_init();
    shell();
}