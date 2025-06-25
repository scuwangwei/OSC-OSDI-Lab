#include <stdint.h>
#include "mini_uart.h"
#include "gpio.h"
#include "base.h"
#include "mailbox.h"
#include "exception.h"

ringBuffer uart_rx_buffer;
ringBuffer uart_tx_buffer;
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

char mini_uart_read_non_block()
{
    while(!((*AUX_MU_LSR_REG) & 0x1)){
        task_dispatcher(0);//keep checking if task need to be executed
    }
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

void mini_uart_read_string_non_block(char *buffer, int max_len)
{
    int i = 0;
    char c;

    while(i < max_len - 1)
    {
        c = mini_uart_read_non_block();//read from register and keep checking task queue

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


/*---------Ring Buffer for Mini UART RX Async-----------*/
void rbuf_init(ringBuffer *rbuf) 
{
    rbuf->head = 0;
    rbuf->tail = 0;
}

int rbuf_is_empty(ringBuffer *rbuf)
{
    return rbuf->head == rbuf->tail;
}

int rbuf_is_full(ringBuffer *rbuf) 
{
    return ((rbuf->head + 1) % RBUF_SIZE) == rbuf->tail;
}

int rbuf_put(ringBuffer *rbuf, char c)
{
    if (rbuf_is_full(rbuf)) return -1; // buffer full

    rbuf->buf[rbuf->head] = c;
    rbuf->head = (rbuf->head + 1) % RBUF_SIZE;
    return 0;
}

int rbuf_get(ringBuffer *rbuf, char *c)
{
    if (rbuf_is_empty(rbuf)) return -1; // buffer empty

    *c = rbuf->buf[rbuf->tail];
    rbuf->tail = (rbuf->tail + 1) % RBUF_SIZE;
    return 0;
}

/*---------Async Mini UART Functions------------*/

//mini uart is type of AUX peripheral,need to enable 29th bit
void aux_irq_enable()
{
    *ENB_IRQs1 |= 1<<29;
}

void aux_irq_disable()
{
    *DISABLE_IRQs1 |= 1<<29;
}

void mini_uart_trans_irq_enable()
{
    *AUX_MU_IER_REG |= 2u; //enable 0b10
}

void mini_uart_trans_irq_disable()
{
    *AUX_MU_IER_REG &= ~(2u); //disable 0b10
}

void mini_uart_recv_irq_enable()
{
    *AUX_MU_IER_REG |= 1u;  //enable 0b01
}

void mini_uart_recv_irq_disable()
{
    *AUX_MU_IER_REG &= ~(1u); // disable 0b10
}

int mini_uart_write_async(const char tmp, int isAsycn)
{
    //check ring buffer and wait it is not full(handler will send the ring buffer data)
    while(rbuf_is_full(&uart_tx_buffer))
    {
        //do nothing
        asm volatile("nop");
    }
    //ring buffer is not full,UART IRQ handler already put the ring buffer data to the transmit FIFO buffer),we can put data to ring buffer now
    int res = rbuf_put(&uart_tx_buffer,tmp);

    //if need to send tx irq,enable it
    if(isAsycn) mini_uart_trans_irq_enable();
    return res;
}


void mini_uart_send_string_async(const char *str)
{
    //send string
    while(*str)
    {
        //string is too long,ring buffer is full,call handler transmit it first
        if(rbuf_is_full(&uart_tx_buffer))
        {
            /*enable transmit irq so that handler will send the all of the data in ring buffer
            (and the handler must be uninterruptable!!!,Otherwise might get race condition with uart_tx_buffer)*/
            mini_uart_trans_irq_enable();
        }
        //call mini_uart_write_async without enable irq,will enable irq after ring buffer is all set
        if(mini_uart_write_async(*(str++),0) < 0)
        {
            return;//something wrong,ring buffer is still full
        }

    }
    mini_uart_trans_irq_enable();//enable trans irq after ring buffer set

}

int mini_uart_read_async(char *c)
{
    mini_uart_recv_irq_enable();//enable recv irq before read ring buffer data
    //here,cpu will not polling receive FIFO buffer,UART IRQ handler will handle the receive FIFO buffer
    while(rbuf_is_empty(&uart_rx_buffer))
    {
        //do nothing until UART irq handler was trigger
        asm volatile("nop");
    }

    //ring buffer is not empty(UART IRQ handler already put the recieve FIFO buffer data to the ring buffer),we can get data from ring buffer now
    int res = rbuf_get(&uart_rx_buffer,c);
    if(*c == '\r') *c = '\n';
    return res;
}

void mini_uart_read_string_async(char *buffer, int max_len)
{
    int i = 0;
    char c;
    //read string
    while(i < max_len - 1)
    {
        if(mini_uart_read_async(&c) < 0) return;//something wrong,ring buffer is still empty
        if(c == '\n') {
            break; // get a cmd
        }

        //echo back with asycn(which mean only write one byte to tx ring buffer and enable tx irq immediately)
        mini_uart_write_async(c,1);
        
        buffer[i++] = c;
    }

    buffer[i] = '\0';// end of a string
    mini_uart_send_string_async("\r\n");
}
