#include "mini_uart.h"
#include "timer.h"
#include "utils.h"
#include "exception.h"


/*--------------Task Queue-----------*/
#define TASK_QUEUE_SIZE 32

typedef struct task {
    void (*func)(void *);
    void *arg;
} task_t;

typedef struct task_queue
{
    task_t task_queue[TASK_PRIORITY_Q_NUM][TASK_QUEUE_SIZE];
    int head[TASK_PRIORITY_Q_NUM];
    int tail[TASK_PRIORITY_Q_NUM];
} multi_lev_task_queue;

multi_lev_task_queue ML_task_queue;

void init_task_queue()
{
    for (int i = 0; i < TASK_PRIORITY_Q_NUM; i++) {
        ML_task_queue.head[i] = 0;
        ML_task_queue.tail[i] = 0;
    }
}


int next_index(int idx) {
    return (idx + 1) % TASK_QUEUE_SIZE;
}

int task_queue_empty(int level) {
    return ML_task_queue.head[level] == ML_task_queue.tail[level];
}

void enqueue_task(void (*func)(void *), void *arg,int priority)
{

    int next = next_index(ML_task_queue.tail[priority]);
    if(next == ML_task_queue.head[priority]) return; //queue full
    int cur = ML_task_queue.tail[priority];
    ML_task_queue.task_queue[priority][cur].func = func;
    ML_task_queue.task_queue[priority][cur].arg = arg;

    ML_task_queue.tail[priority] = next;
}

void task_dispatcher(int prior_level)
{
    int cur_lev_head;
    while (prior_level >= 0)
    {
        if (task_queue_empty(prior_level)) 
        {
            prior_level--;
            break;
        }
        cur_lev_head = ML_task_queue.head[prior_level];
        task_t task = ML_task_queue.task_queue[prior_level][cur_lev_head];
        cur_lev_head = next_index(cur_lev_head);
        ML_task_queue.head[prior_level] = cur_lev_head;
        task.func(task.arg);

    }
}

//read and print registers info
void print_register_info(void *arg)
{
    unsigned long spsr, elr, esr;
    asm volatile("mrs %0, spsr_el1" : "=r"(spsr));
    asm volatile("mrs %0, elr_el1"  : "=r"(elr));
    asm volatile("mrs %0, esr_el1"  : "=r"(esr));

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

    mini_uart_send_string("Warining:This user program will hang,so you need to restart the system\r\n");

}

/*-------------------------------Handlers-----------------------*/

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
        enqueue_task(expired_event->callback,expired_event->data,1);
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
    else core_timer_set_timeout(1);

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
        //move receive FIFO buffer data to ring buffer
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


void lower_el_aarch64_sync_c()
{
    disable_irq();
    //print SPSR_EL1 ELR_EL1 ESR_EL1
    enqueue_task(print_register_info,NULL,0);
    task_dispatcher(0);//dispatch task with irq enabled to support nested irq
    
    enable_irq();
}

void lower_el_aarch64_irq_c()
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
    task_dispatcher(TASK_MAX_PRIORITY);

    enable_irq();

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


/*-------------------Init----------------*/
void irq_init()
{
    init_task_queue();
    enable_irq();
    core_timer_enable();
}
