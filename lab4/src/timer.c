#include "timer.h"
#include "mini_uart.h"
#include "utils.h"
void enable_irq()
{
    asm volatile("msr DAIFClr, 0xf");
}

void disable_irq()
{
    asm volatile("msr DAIFSet, 0xf");
}

void core_timer_set_timeout(unsigned long sec)
{
    unsigned long frq;

    // get frequency(normally 19.2MHz)
    asm volatile("mrs %0, cntfrq_el0" : "=r"(frq));

    // set timer
    unsigned long interval = frq * sec;

    asm volatile("msr cntp_tval_el0, %0" :: "r"(interval));
}
void core_timer_enable() 
{
    // enable core timer
    asm volatile("msr cntp_ctl_el0, %0" :: "r"(1));

    unsigned long frq;

    // get frequency(normally 19.2MHz)
    asm volatile("mrs %0, cntfrq_el0" : "=r"(frq));

    // set timer
    unsigned long interval = frq * 1;

    asm volatile("msr cntp_tval_el0, %0" :: "r"(interval));
    
    //unmask timer interrupt
    *CORE0_TIMER_IRQ_CTRL = (1 << 1);
}

unsigned long get_current_time()
{
    unsigned long frq, count;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(frq));
    asm volatile("mrs %0, cntpct_el0" : "=r"(count));
    return count / frq; // sec
}

unsigned long get_freq()
{
    unsigned long frq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(frq));
    return frq;
}

unsigned long read_cntpct_el0()
{
    unsigned long cnt;
    asm volatile("mrs %0, cntpct_el0" : "=r" (cnt));
    return cnt;
}

void core_timer_set_timeout_ticks(unsigned long ticks)
{
    asm volatile("msr cntp_tval_el0, %0" :: "r"(ticks));
}

/*----------------Add Timer API---------------*/
timer_event_t timer_pool[MAX_TIMER_EVENTS]; // static tiemr pool
int timer_pool_used[MAX_TIMER_EVENTS];      // mark slot is used or not
timer_event_t *timer_queue = 0; // sorted linked list head

/*alooc timer slot*/
timer_event_t *timer_alloc()
{
    //find an empty slot
    for (int i = 0; i < MAX_TIMER_EVENTS; i++) {
        if (!timer_pool_used[i]) {
            timer_pool_used[i] = 1;
            return &timer_pool[i];
        }
    }
    return 0; // no space left
}

/*free timer slot*/
void timer_free(timer_event_t *event)
{
    int index = event - timer_pool;
    if (index >= 0 && index < MAX_TIMER_EVENTS)
        timer_pool_used[index] = 0;
}

void add_timer(void (*callback)(void *), void *data, unsigned int delay_secs)
{
    disable_irq();//disable incase of race condition

    //allocate new timer event
    timer_event_t *new_event = timer_alloc();
    if(!new_event)
    {
        mini_uart_send_string("No Timer Event Slot Space left\r\n");
        return;
    }

    //initialize timer event
    unsigned long cur_ticks = read_cntpct_el0();
    unsigned long frq = get_freq();
    new_event->expire_time = cur_ticks + ((unsigned long)delay_secs) * frq;
    new_event->callback = callback;
    new_event->data = data;
    new_event->next = 0;


    //if new event is first timer event or it has shortest expire time,add it in front of the queue
    if(!timer_queue || new_event->expire_time < timer_queue->expire_time)
    {
        new_event->next = timer_queue;
        timer_queue = new_event;
        core_timer_set_timeout_ticks(new_event->expire_time - cur_ticks);
    }
    //else add it in the middle of the queue
    else
    {
        //add the new event by comparing their expire times
        timer_event_t *cur_event = timer_queue;
        while(cur_event->next && cur_event->next->expire_time < new_event->expire_time)
        {
            cur_event = cur_event->next;
        }
        new_event->next = cur_event->next;
        cur_event->next = new_event;
    }

    enable_irq();
}

/*Message Pool for Non-Dyanamic Allocation Enviroment*/
char msg_pool[MAX_MESSAGES][MAX_MSG_LEN];
int msg_pool_used[MAX_MESSAGES];

char *alloc_msg_slot(const char *src) {
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (!msg_pool_used[i]) {
            msg_pool_used[i] = 1;
            simple_strncpy(msg_pool[i], src, MAX_MSG_LEN - 1);
            msg_pool[i][MAX_MSG_LEN - 1] = '\0';
            return msg_pool[i];
        }
    }
    return 0;
}

void free_msg_slot(char *ptr) {
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (msg_pool[i] == ptr) {
            msg_pool_used[i] = 0;
            return;
        }
    }
}

/*-------------------------Callback Function------------------------*/
void print_msg_callback(void *msg)
{
    //print message
    mini_uart_send_string((const char*)msg);
    mini_uart_send_string("\r\n");
    mini_uart_send_string("#");
    free_msg_slot(msg);//free message slot
}

void print_boot_time(void *unused)
{
    unsigned long seconds = get_current_time();
    mini_uart_send_string("Seconds since boot: ");
    mini_uart_send_string(int2char((int)seconds));
    mini_uart_send_string("\r\n");
    mini_uart_send_string("#");
}