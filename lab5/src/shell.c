#include "mini_uart.h"
#include "pm.h"
#include "mailbox.h"
#include "utils.h"
#include "cpio.h"
#include "allocator.h"
#include "dtb.h"
#include "timer.h"
#include "exception.h"

typedef enum {
    CMD_HELP,
    CMD_HELLO,
    CMD_INFO,
    CMD_REBOOT,
    CMD_LS,
    CMD_CAT,
    CMD_DTB_TEST,
    CMD_BTIME,
    CMD_SET_TIMER,
    CMD_ALLOC,
    CMD_FREE,
    CMD_PFREELIST,
    CMD_DMEMORY_ALLOC,
    CMD_LOAD_PROGRAM,
    CMD_ASYNC_UART,
    CMD_UNKNOWN
} command_id_t;

#define CMD_BUFFER_SIZE 128
#define MAX_READ_BUFFER 64
char cmd_buffer[CMD_BUFFER_SIZE];
extern void* _dtb_addr;
void *mem_addr = NULL;

stack_t my_stack;

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

command_id_t get_command_id(const char *cmd) {
    if (!strings_compare(cmd, "help")) return CMD_HELP;
    else if (!strings_compare(cmd, "hello")) return CMD_HELLO;
    else if (!strings_compare(cmd, "info")) return CMD_INFO;
    else if (!strings_compare(cmd, "reboot")) return CMD_REBOOT;
    else if (!strings_compare(cmd, "ls")) return CMD_LS;
    else if (!strings_compare(cmd, "cat")) return CMD_CAT;
    else if (!strings_compare(cmd, "dtb_test")) return CMD_DTB_TEST;
    else if (!strings_compare(cmd, "btime")) return CMD_BTIME;
    else if (!strcmp(cmd, "setTimer", 8)) return CMD_SET_TIMER;
    else if (!strcmp(cmd, "alloc", 5)) return CMD_ALLOC;
    else if (!strings_compare(cmd, "free")) return CMD_FREE;
    else if (!strings_compare(cmd, "pfreeList")) return CMD_PFREELIST;
    else if (!strings_compare(cmd, "dmemoryAlloc")) return CMD_DMEMORY_ALLOC;
    else if (!strings_compare(cmd, "load_program")) return CMD_LOAD_PROGRAM;
    else if (!strings_compare(cmd, "async_uart")) return CMD_ASYNC_UART;
    else return CMD_UNKNOWN;
}

/*show all command to shell via mini uart*/
void show_all_command()
{
    mini_uart_send_string("Supported commands:\r\n");
    mini_uart_send_string("help - Show this help message\r\n");
    mini_uart_send_string("hello - Show Hello World\r\n");
    mini_uart_send_string("info - Show board revision and memory info\r\n");
    mini_uart_send_string("reboot - Reboot the system,This only works on real rpi machine\r\n");
    mini_uart_send_string("ls - list all ramdisk files\r\n");
    mini_uart_send_string("cat - get ramdisk file\r\n");
    mini_uart_send_string("load_program - load a program from initramfs and execute it\r\n");
    mini_uart_send_string("current executable file in initramfs:\r\n");
    mini_uart_send_string("     user.img:get spsr_el1,elr_el1 and esr_el1 5 times and hang\r\n");
    mini_uart_send_string("dtb_test - parse dtb and get ramfs address\r\n");
    mini_uart_send_string("btime - set a timer with 1 second timeout,it will print the seconds since boot\r\n");
    mini_uart_send_string("setTimer {message} {seconds} - set a timer,which will print the message when timeout\r\n");
    mini_uart_send_string("async_uart - activate async uart,you can type messages via async uart\r\n");
    mini_uart_send_string("pfreeList - print the free list in buddy system\r\n");
    mini_uart_send_string("alloc {size} - allocate a contigous memory block,and add the variable address to the stack\r\n");
    mini_uart_send_string("free - free the variable which at the top of the stack\r\n");
    mini_uart_send_string("dmemoryAlloc - allocate a block for a char*,Assigning some chars to this variable',and add the variable address to the stack");

}


/*parse command and response accroding each command*/
void parse_command(const char *cmd_raw) {
    const char *cmd = cmd_raw;
    command_id_t cmd_id = get_command_id(cmd);

    switch (cmd_id) {
        case CMD_HELP:
            show_all_command();
            break;

        case CMD_HELLO:
            mini_uart_send_string("Hello World");
            break;

        case CMD_INFO:
            get_board_revision();
            mini_uart_send_string("\r\n");
            get_memory_information();
            break;

        case CMD_REBOOT:
            mini_uart_send_string("Rebooting...");
            reboot();
            break;

        case CMD_LS:
            cpio_ls();
            break;

        case CMD_CAT:
            cpio_cat();
            break;

        case CMD_DTB_TEST:
            fdt_traverse(_dtb_addr, get_ramfs_addr);
            break;

        case CMD_BTIME:
            set_btime_timer();
            break;

        case CMD_SET_TIMER:
            cmd += 8;  // skip "setTimer"
            if (*cmd != ' ') {
                mini_uart_send_string("input format error\r\n");
                break;
            }
            cmd++;
            unsigned int seconds;
            char msg[MAX_MSG_LEN];
            if (parse_msg_secs(cmd, msg, (int)MAX_MSG_LEN, &seconds) < 0) {
                mini_uart_send_string("Input format error\r\n");
            } else {
                set_message_timer(msg, &seconds);
            }
            break;

        case CMD_ALLOC:
            cmd += 5;
            if (*cmd != ' ') {
                mini_uart_send_string("input format error\r\n");
            } else {
                cmd++;
                int size;
                if (*cmd == '\0' || parse2int(cmd, &size) < 0) {
                    mini_uart_send_string("input format error\r\n");
                } else {
                    mem_addr = malloc(size);
                    mini_uart_send_string("Memory allocated, add the address to stack\r\n");
                    if (!stack_push(&my_stack, mem_addr)) {
                        mini_uart_send_string("Stack full, cannot add address to stack\r\n");
                        free(mem_addr);
                    }
                }
            }
            break;

        case CMD_FREE: {
            void *tmp = stack_pop(&my_stack);
            if (!tmp) {
                mini_uart_send_string("No address in the stack\r\n");
            } else {
                free(tmp);
            }
            break;
        }

        case CMD_PFREELIST:
            print_free_list();
            break;

        case CMD_DMEMORY_ALLOC: {
            char *testMem = (char *)malloc(40);
            const char *text = "Test Dynamic Memory Allocator";
            for (int i = 0; i < 40 && text[i]; ++i)
                testMem[i] = text[i];
            testMem[39] = '\0';

            if (!stack_push(&my_stack, (void *)testMem)) {
                mini_uart_send_string("Stack full,cannot add address to stack\r\n");
                free(testMem);
            } else {
                mini_uart_send_string("\r\n");
                mini_uart_send_string("The System just allocated a block for the string: \r\n");
                mini_uart_send_string(testMem);
                mini_uart_send_string("\r\n");
            }
            break;
        }

        case CMD_LOAD_PROGRAM:
            cpio_load_program();
            break;

        case CMD_ASYNC_UART:
            mini_uart_send_string("---------------Start Async Mini Uart Test--------------\r\n");
            test_asycn_mini_uart();
            mini_uart_send_string("----------------Async Mini UART Test End---------------");
            break;

        default:
            mini_uart_send_string("Unknown command");
            break;
    }

    mini_uart_send_string("\r\n");
}

/*implement shell function which read string from mini uart and call parse_command() function*/
void shell()
{
    stack_init(&my_stack);
    while(1)
    {
        task_dispatcher(0);//execute all task if task queue is not empty
        mini_uart_send_string("#");//echo a # to console
        mini_uart_read_string_non_block(cmd_buffer,CMD_BUFFER_SIZE);//wait for host send command and keep checking task queue
        parse_command(cmd_buffer);//parse command
    }

}

