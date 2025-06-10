#include "mini_uart.h"
#include "pm.h"

#define CMD_BUFFER_SIZE 128
char cmd_buffer[CMD_BUFFER_SIZE];

/*reboot function*/
void reboot()
{
    // Set timeout (at least 10 ticks)
    *PM_WDOG = PM_WDOG_MAGIC | 10;

    // Trigger full reset
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_WRCFG_FULL_RESET;

    while (1) { } // Hang until reset
}
/*eturn value
    0 : same
    positive int : different and s1 len > s2 len
    negative int : different and s1 len < s2 len    
*/
int strings_different(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2; // return difference if not equal
        }
        s1++;
        s2++;
    }
    return *s1 - *s2; // handles cases where one string is longer
}

/*show all command to shell via mini uart*/
void show_all_command()
{
    mini_uart_send_string("Supported commands:\r\n");
    mini_uart_send_string("help - Show this help message\r\n");
    mini_uart_send_string("hello - Show Hello World\r\n");
    mini_uart_send_string("info - Show board revision and memory info\r\n");
    mini_uart_send_string("reboot - Reboot the system");

}

/*parse command and response accroding each command*/
void parse_command(const char *cmd)
{
    if(!strings_different(cmd,"hello"))
    {
        mini_uart_send_string("Hello World");
    }
    else if(!strings_different(cmd,"help"))
    {
        show_all_command();
    }
    else if(!strings_different(cmd,"info"))
    {
        get_board_revision();
        mini_uart_send_string("\r\n");
        get_memory_information();
    }
    else if(!strings_different(cmd,"reboot"))
    {
        mini_uart_send_string("Rebooting...");
        reboot();
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
    // initialize mini uart serial port
    mini_uart_init();
    while(1)
    {
        mini_uart_send_string("#");
        mini_uart_read_string(cmd_buffer,CMD_BUFFER_SIZE);
        parse_command(cmd_buffer);
    }

}

