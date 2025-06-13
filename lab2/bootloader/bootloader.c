#include "mini_uart.h"

#define MAX_READ_BUFFER 64
#define KERNEL_ADDRESS 0x80000

typedef void (*kernel_entry_t)(void *);

char *const kernel_address = (char *)KERNEL_ADDRESS;
extern void* _dtb_addr;

// jump to kernel,and pass dtb_addr to kernel(IMPORTANT: because somehow x0 will be modified after executing bootloader program,so I have to save dtb address in var _dtb_addr first,and pass it at the end of bootloader program)
void jump_to_kernel(void *dtb_addr, void *kernel_addr) {
    kernel_entry_t kernel_entry = (kernel_entry_t)kernel_addr;

    mini_uart_send_string("Going to jump to kernel and pass dtb address to x0\r\n");
    mini_uart_send_hex((unsigned int)_dtb_addr);
    kernel_entry(dtb_addr);
}

//parse char to int
int parse2int(const char *buffer, unsigned int *value) {
    unsigned int result = 0;
    for (int i = 0; buffer[i] != '\0'; ++i) {
        if (buffer[i] >= '0' && buffer[i] <= '9') {
            result = result * 10 + (buffer[i] - '0');
        } else {
            return -1;  // error
        }
    }
    *value = result;
    return 0;  // sucess
}

//recieve image raw data and put it to 0x80000
void recieve_and_load_img()
{
    //get kernel image size
    char buffer [MAX_READ_BUFFER];
    char c;
    int i = 0;
get_size:
    mini_uart_send_string("Enter kernel image size: ");
    while(1)
    {
        c = mini_uart_read();
        mini_uart_write(c); // echo back
        if(c == '\n') break;
        if (i < MAX_READ_BUFFER - 1) buffer[i++] = c;
    }
    mini_uart_send_string("\r\n");//new line for host terminal
    buffer[i] = '\0';
    unsigned int img_size = 0;
    if(parse2int(buffer,&img_size) < 0)
    {
        mini_uart_send_string("Input Error\r\n");
        goto get_size;
    }
    else
    {
        //declare a cur_add so that i can put raw data
        char *cur_add = kernel_address;
        mini_uart_send_string("Ready to recieve kernel image, You can send kernel image now\r\n");
       
        for (unsigned int j = 0; j < img_size; ++j) {
            kernel_address[j] = mini_uart_read_raw();//read raw data       
        }
        mini_uart_send_string("Kernel image recieved\r\n");

    }

}
void bootloader_main()
{
    mini_uart_init();
    mini_uart_send_string("Hello from bootloader program\r\n");
    //recieve kernel image from UART and put it at 0x80000
    recieve_and_load_img();
    // Jump to the new kernel image adress
    jump_to_kernel(_dtb_addr, (void *)KERNEL_ADDRESS);
}