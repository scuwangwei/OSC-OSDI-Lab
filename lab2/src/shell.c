#include "mini_uart.h"
#include "pm.h"
#include "mailbox.h"
#include "utils.h"
#include "cpio.h"
#include "alloca.h"
#include "dtb.h"

#define CMD_BUFFER_SIZE 128
char cmd_buffer[CMD_BUFFER_SIZE];
extern void* _dtb_addr;

/*reboot function*/
void reboot()
{
    // Set timeout (at least 10 ticks)
    *PM_WDOG = PM_WDOG_MAGIC | 10;

    // Trigger full reset
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_WRCFG_FULL_RESET;

    while (1) { } // Hang until reset
}

/*show all command to shell via mini uart*/
void show_all_command()
{
    mini_uart_send_string("Supported commands:\r\n");
    mini_uart_send_string("help - Show this help message\r\n");
    mini_uart_send_string("hello - Show Hello World\r\n");
    mini_uart_send_string("info - Show board revision and memory info\r\n");
    mini_uart_send_string("alloc_test - keep allocating memory until out of memory\r\n");
    mini_uart_send_string("reboot - Reboot the system,This only works on real rpi machine\r\n");
    mini_uart_send_string("ls - list all ramdisk files\r\n");
    mini_uart_send_string("cat - get ramdisk file\r\n");
    mini_uart_send_string("dtb_test - parse dtb and get ramfs address");

}

/*parse command and response accroding each command*/
void parse_command(const char *cmd)
{
    if(!strings_compare(cmd,"hello"))
    {
        mini_uart_send_string("Hello World");
    }
    else if(!strings_compare(cmd,"help"))
    {
        show_all_command();
    }
    else if(!strings_compare(cmd,"info"))
    {
        get_board_revision();
        mini_uart_send_string("\r\n");
        get_memory_information();
    }
    else if(!strings_compare(cmd,"reboot"))
    {
        mini_uart_send_string("Rebooting...");
        reboot();
    }
    else if(!strings_compare(cmd,"ls"))
    {
        cpio_ls();
    }
    else if(!strings_compare(cmd,"cat"))
    {
        cpio_cat();
    }
    else if(!strings_compare(cmd,"alloc_test"))
    {
        test_allocator_exhaustion();
    }
    else if(!strings_compare(cmd,"dtb_test"))
    {
        fdt_traverse(_dtb_addr,get_ramfs_addr);
    }
    else
    {
        mini_uart_send_string("Unknown command");
    }
    mini_uart_send_string("\r\n");// new line


}

/*implement shell function which read string from mini uart and call parse_command() function*/
void shell()
{
    while(1)
    {
        mini_uart_send_string("#");//echo a # to console
        mini_uart_read_string(cmd_buffer,CMD_BUFFER_SIZE);
        parse_command(cmd_buffer);
    }

}

