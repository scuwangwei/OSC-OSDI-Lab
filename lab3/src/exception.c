#include "mini_uart.h"
#include "timer.h"
#include "utils.h"
#include "exception.h"

/*Core Timer Interrupt Handler,executed when time expired*/
void core_timer_irq_handler() {
    //mini_uart_send_string("core timer irq called\r\n");
    extern timer_event_t timer_pool[MAX_TIMER_EVENTS];
    extern int timer_pool_used[MAX_TIMER_EVENTS];
    extern timer_event_t *timer_queue;
    unsigned long cur_ticks = read_cntpct_el0();
    //traverse timer queue,and execute call back for those which time expired
    while(timer_queue && timer_queue->expire_time <= cur_ticks)
    {

        timer_event_t *expired_event = timer_queue;
        timer_queue = timer_queue->next;
        expired_event->callback(expired_event->data);
        timer_free(expired_event);
    }

    //set next timeout
    if(timer_queue)
    {

        unsigned long delay = timer_queue->expire_time - cur_ticks;
        //incase next timeout less than 0,so at lease delay tick = 10
        if(delay <=0 ) delay = 10;
        core_timer_set_timeout_ticks(delay);
        return;
    }
    core_timer_set_timeout(1);

}

/*UART Interrupt Handler for Async Read/Write Mini UART*/
void mini_uart_irq_handler()
{
    extern ringBuffer uart_rx_buffer;
    extern ringBuffer uart_tx_buffer;
    
    //if [2:1] = 0b10 then receive FIFO has data
    int isRX = (*AUX_MU_IIR_REG & 0x4);
    //if [2:1] = 0b01 then transmit holding empty
    int isTX = (*AUX_MU_IIR_REG & 0x2);
    if(isRX)
    {
        //move receive FIFO buffer data to ring buffer,
        char c = (char)(*AUX_MU_IO_REG);
        rbuf_put(&uart_rx_buffer, c);
        //read finished,disable recieve IRQ
        mini_uart_recv_irq_disable();
    }
    else if(isTX)
    {
        //keep sending data to trans FIFO buffer and check if it is empty
        while(*AUX_MU_LSR_REG & 0x20)
        {
            char c;
            if(rbuf_get(&uart_tx_buffer,&c) < 0)
            {
                //ring bufer empty,nothing to transmit
                break;
            }
            *AUX_MU_IO_REG = c;//put data from ring buffer to transmit FIFO buffer
            
        }
        //disable trans IRQ
        mini_uart_trans_irq_disable();

    }
}


void lower_el_aarch64_sync_c(unsigned long spsr, unsigned long elr, unsigned long esr)
{
    //print SPSR_EL1 ELR_EL1 ESR_EL1
    mini_uart_send_string("\r\n[Exception Handler]\r\n");
    mini_uart_send_string("SPSR_EL1: ");
    mini_uart_send_hex(spsr);
    mini_uart_send_string("\r\n");

    mini_uart_send_string("ELR_EL1: ");
    mini_uart_send_hex(elr);
    mini_uart_send_string("\r\n");

    mini_uart_send_string("ESR_EL1: ");
    mini_uart_send_hex(esr);
    mini_uart_send_string("\r\n");
}


void el_curr_el_spx_irq_c()
{
    disable_irq();//turn of irq in case race condition
     // get irq source
    if (*CORE0_INTERRUPT_SOURCE & (1 << 1)) {
        // Core timer interrupt
        core_timer_irq_handler();
    }
    if (*IRQ_PENDING_1 & (1 << 29)) {
        // Mini UART interrupt (bit 29 in IRQ_PENDING_1)
        mini_uart_irq_handler();
    }
    enable_irq();
}
