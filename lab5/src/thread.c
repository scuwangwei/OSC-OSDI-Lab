#include "thread.h"
#include "mini_uart.h"
#include "utils.h"
// run queue head
static thread_t* run_queue = NULL;
// current thread pointer (from TPIDR_EL1)
extern thread_t* get_current();
//context switching
extern void switch_to(thread_t* prev, thread_t* next);
unsigned long next_thread_id = 1;

void enqueue_thread(thread_t* t)
{
    t->next = NULL;
    if (!run_queue) {
        run_queue = t;
    } else {
        thread_t* cur = run_queue;
        while (cur->next) cur = cur->next;
        cur->next = t;
    }
}

void remove_thread(thread_t* t)
{
    if (run_queue == t) {
        run_queue = t->next;
        return;
    }
    thread_t* prev = run_queue;
    while (prev && prev->next != t) prev = prev->next;
    if (prev) prev->next = t->next;
}

void thread_create(void (*func)())
{
    mini_uart_send_string("-----------------------Creating a Thread,Allocating Memory For It-------------------\r\n");
    //allocate memory for thread struct
    thread_t* t = (thread_t*)malloc(sizeof(thread_t));
    //allocate memory for thread stack
    t->stack = malloc(STACK_SIZE);

    // Initialize context
    for (int i = 0; i < 13; i++) t->context[i] = 0;
    t->context[11] = (uint64_t)func; // lr
    t->context[12] = (uint64_t)t->stack + STACK_SIZE; // sp

    t->id = next_thread_id++;
    t->state = THREAD_READY;
    mini_uart_send_string("Thread Address at: ");
    mini_uart_send_hex((unsigned int)t);
    mini_uart_send_string("\r\n");
    enqueue_thread(t);
    mini_uart_send_string("-----------------------Thread Created-------------------\r\n");
}

void schedule()
{
    thread_t* current = get_current();
    thread_t* next = run_queue;

    mini_uart_send_string("Current thread address: ");
    mini_uart_send_hex((unsigned int)current);
    mini_uart_send_string("\r\n");


    mini_uart_send_string("Next thread address: ");
    mini_uart_send_hex((unsigned int)next);
    mini_uart_send_string("\r\n");
    if (!next || next == current) return;

    // Move current to end if still alive
    if (current && current->state == THREAD_READY) {
        remove_thread(current);
        enqueue_thread(current);
    }

    next = run_queue;
    remove_thread(next);

    if (current != next) {
        switch_to(current, next);
    }
}

void kill_zombies()
{
    thread_t* cur = run_queue;
    thread_t* prev = NULL;

    while (cur) {
        if (cur->state == THREAD_DEAD) {
            if (prev) prev->next = cur->next;
            else run_queue = cur->next;

            free(cur->stack);
            thread_t* temp = cur;
            cur = cur->next;
            free(temp);
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
}

void idle()
{
    while (1) {
        kill_zombies();
        schedule();
    }
}

void thread_exit()
{
    thread_t* current = get_current();
    current->state = THREAD_DEAD;
    schedule(); // Yield to next thread
    while (1);  // Should never return
}
