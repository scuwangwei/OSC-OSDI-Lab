#include "mini_uart.h"
#include "pm.h"
#include "mailbox.h"
#include "utils.h"
#include "cpio.h"
#include "alloca.h"
#include "dtb.h"
#include "timer.h"
#include "exception.h"

#define CMD_BUFFER_SIZE 128
#define MAX_READ_BUFFER 64
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

/*test async mini uart function*/
void test_asycn_mini_uart()
{
    aux_irq_enable();
    mini_uart_send_string("Try to type some message(Max len:300)\r\n");

    char test_buffer[301];

    extern ringBuffer uart_rx_buffer;
    extern ringBuffer uart_tx_buffer;

    //init ring buffers
    rbuf_init(&uart_rx_buffer);
    rbuf_init(&uart_tx_buffer);

    //async read/write mini uart
    mini_uart_read_string_async(test_buffer,301);
    mini_uart_send_string_async("You just typed a message via async mini uart:\r\n");
    mini_uart_send_string_async(test_buffer);
    mini_uart_send_string_async("\r\n");

    //disable after async transmit send all data
    while(!rbuf_is_empty(&uart_tx_buffer))
    {
        asm volatile("nop");
    }
    aux_irq_disable;

}

void set_message_timer(const char *msg, unsigned int *sec)
{
    char *msg_slot = alloc_msg_slot(msg);
    if (msg_slot) {
        add_timer(print_msg_callback, msg_slot, *sec);
    }
    else
    {
        mini_uart_send_string("No message slot space left\r\n");
    }
}

void set_btime_timer()
{
    add_timer(print_boot_time,NULL,1);
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
    mini_uart_send_string("load_program - load a program from initramfs and execute it\r\n");
    mini_uart_send_string("current executable file in initramfs:\r\n");
    mini_uart_send_string("     user.img:get spsr_el1,elr_el1 and esr_el1 5 times and hang\r\n");
    mini_uart_send_string("dtb_test - parse dtb and get ramfs address\r\n");
    mini_uart_send_string("btime - set a timer with 1 second timeout,it will print the seconds since boot\r\n");
    mini_uart_send_string("setTimer {message} {seconds} - set a timer,which will print the message when timeout\r\n");
    mini_uart_send_string("async_uart - activate async uart,you can type messages via async uart");

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
    else if (!strings_compare(cmd,"btime"))
    {
        set_btime_timer();
    }
    else if (!strcmp(cmd,"setTimer",8))
    {
        cmd += 8;//skip "setTImer"
        if(*cmd != ' ')
        {
            mini_uart_send_string("input format error\r\n");
        }
        else{
            cmd++;
            unsigned int seconds;
            char msg[MAX_MSG_LEN];
            if(parse_msg_secs(cmd,msg,(int)MAX_MSG_LEN,&seconds) < 0)
            {
                    mini_uart_send_string("Input format error\r\n");
            }
            else
            {
                set_message_timer(msg,&seconds);
            }

        }

    }
    else if (!strings_compare(cmd,"load_program"))
    {
        cpio_load_program();
    }
    else if (!strings_compare(cmd,"async_uart"))
    {
        mini_uart_send_string("---------------Start Async Mini Uart Test--------------\r\n");
        test_asycn_mini_uart();
        mini_uart_send_string("----------------Async Mini UART Test End---------------");
    }
    else{
        mini_uart_send_string("Unknown command");
    }
    mini_uart_send_string("\r\n");// new line


}

/*implement shell function which read string from mini uart and call parse_command() function*/
void shell()
{
    while(1)
    {
        task_dispatcher(TASK_MAX_PRIORITY);//execute task if task queue is not empty
        mini_uart_send_string("#");//echo a # to console
        mini_uart_read_string_non_block(cmd_buffer,CMD_BUFFER_SIZE);//wait for host send command and keep checking task queue
        parse_command(cmd_buffer);//parse command
    }

}

