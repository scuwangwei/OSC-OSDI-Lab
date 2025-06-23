#ifndef TIMER_H
#define TIMER_H

#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int *)(0x40000040))
#define MAX_TIMER_EVENTS 32
#define MAX_MESSAGES 32
#define MAX_MSG_LEN 64

typedef struct timer_event {
    unsigned long expire_time; //timeout
    void (*callback)(void *); //call back function
    void *data; //call back function parameters
    struct timer_event *next; //next timer event
} timer_event_t;

void core_timer_set_timeout(unsigned long sec);
void core_timer_enable() ;
void enable_irq();
unsigned long get_current_time();
unsigned long get_freq();
unsigned long read_cntpct_el0();
void core_timer_set_timeout_ticks(unsigned long ticks);
timer_event_t *timer_alloc();
void timer_free(timer_event_t *event);
void add_timer(void (*callback)(void *), void *data, unsigned int delay_secs);
void print_msg_callback(void *msg);
void print_boot_time();

#endif
