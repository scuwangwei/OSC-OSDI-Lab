#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stddef.h>
#define STACK_SIZE 4096
#define MAX_THREADS 32

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_DEAD
} thread_state_t;

typedef struct thread {
    uint64_t context[12]; // x19~x28, fp, lr, sp (64 bits)
    void* stack;
    int id;
    thread_state_t state;
    struct thread* next; // for run queue
} thread_t;

void thread_create(void (*func)());
void schedule();
void kill_zombies();
void idle();
void thread_exit();

#endif