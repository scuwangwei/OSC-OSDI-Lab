#include <stdint.h>
#include "mini_uart.h"
#include "gpio.h"
#include "base.h"
#include "mailbox.h"

void mini_uart_init()
{
    /*---------set alt for gpio 14,15----------------*/
    unsigned int tmp;
    // from document,gpio 14 alt and 15 alt was determined at 14~12 bits and 17~15 bits respectively in GPFSEL1 register
    tmp = *GPFSEL1;
    tmp &= ~(7u<<12);//clean 000 for gpio 14
    tmp |= 2u<<12;//set 010(alt5) for gpio 14
    tmp &= ~(7u<<15);//clean 000 for gpio 15
    tmp |= 2u<<15; //set 010(alt5) for gpio 15
    *GPFSEL1 = tmp;

    //disable pull up/low for gpio 14/15
    *GPPUD = 0u;// set 00 means disable and then wait
    unsigned int wait_clock = 150u;
    while(wait_clock--){
        asm volatile("nop");
    }
    wait_clock = 150u;
    *GPPUDCLK0 = 3u<<14;//send signal and then wait
    while(wait_clock--){
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0u;//stop send signal

    /*-----------set aux mini uart-----------------*/

    *AUX_ENABLE = 1u;//the lowest bit refer mini uart enable,and it also means we can access the mini uart registers
    *AUX_MU_CNTL_REG = 0u;//disable features like CTS RTS AUTO flow control and transmitter/reciever
    *AUX_MU_IER_REG = 0u;//currently I don't need interrupt;
    *AUX_MU_LCR_REG = 3u;//set data size to 8 bit
    *AUX_MU_MCR_REG = 0u;//currently I don't need auto flow control
    *AUX_MU_BAUD = 270u;//set baud rate to 115200
    *AUX_MU_IIR_REG = 6u;//clear Tx/Rx FIFO buffer,make sure they are clean
    *AUX_MU_CNTL_REG = 3u;//enable transmitter and reciever

}

char mini_uart_read()
{
    while(!((*AUX_MU_LSR_REG) & 0x1)){} //if data not ready,wait until ready(from document,lowest bit refers it's ready or not)
    char tmp = (*AUX_MU_IO_REG) & 0xFF;
    if(tmp == '\r') tmp = '\n';// /r means newline in keyboard
    return tmp;
}

void mini_uart_write(char tmp)
{
    while(!((*AUX_MU_LSR_REG) & 0x20)){} //if transmitter cannot accept at least one byte,wait(from document,5th bit referes trasmitter empty)
    *AUX_MU_IO_REG = tmp;//write to register
}

void mini_uart_send_string(const char *str)
{
    while(*str)
    {
        mini_uart_write(*str++);
    }
}

void mini_uart_read_string(char *buffer, int max_len)
{
    int i = 0;
    char c;

    while(i < max_len - 1)
    {
        c = mini_uart_read();//read from register

        if(c == '\n') {
            break; // get a cmd
        }
        
        mini_uart_write(c); //echo back

        buffer[i++] = c;
    }

    buffer[i] = '\0';// end of a string
    mini_uart_send_string("\r\n");
}


void mini_uart_send_hex(unsigned int num)
{
    mini_uart_send_string("0x");
    for (int i = 28; i >= 0; i -= 4)
    {
        int n = (num >> i) & 0xF;
        n += n > 9 ? 0x37 : 0x30;
        mini_uart_write(n);
    }
}

char mini_uart_read_raw()
{
    while (!(*(AUX_MU_LSR_REG) & 0x1)){}
    char tmp = (*AUX_MU_IO_REG) & 0xFF;
    return tmp;
}