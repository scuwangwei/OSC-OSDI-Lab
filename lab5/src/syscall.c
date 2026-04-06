#include "thread.h"
#include "trapframe.h"
#include "mini_uart.h"
#include "syscall.h"


//get pid (thread id)
int sys_getpid()
{
    //get the current thread struct (tpidr_el1)
    thread_t *current = get_current();
    //return pid (thread id)
    return current->id;
}

size_t sys_uart_read()
{

}

int sys_fork(struct trap_frame *tf)
{

}

size_t sys_uart_write(const char buf[], size_t size)
{
    size_t bytes_written = 0;

    // Loop through the user-provided buffer
    for (size_t i = 0; i < size; i++) {
        // output via mini uart
        mini_uart_write(buf[i]); 
        bytes_written++;
    }

    // Return the total number of bytes successfully written.
    // This value goes back into tf->x[0] and is returned to the user program.
    return bytes_written;
}

void syscall_handler(struct trap_frame *tf)
{
    //get system call number
    uint64_t syscall_no = tf->x[8];
    switch (syscall_no) {
        case 0:
            tf->x[0] = sys_getpid();
            break;
        // case 1:
        //     tf->x[0] = sys_uart_read((char*)tf->x[0], tf->x[1]);
        //     break;
        case 2:
            tf->x[0] = sys_uart_write((char*)tf->x[0], tf->x[1]);
            break;
        case 3:
            tf->x[0] = sys_exec((char*)tf->x[0], (char**)tf->x[1]);
            break;
        case 4:
            tf->x[0] = sys_fork(tf);
            break;
        // case 5:
        //     sys_exit(tf->x[0]);
        //     break;
        // case 6:
        //     tf->x[0] = sys_mbox_call(tf->x[0], (unsigned int*)tf->x[1]);
        //     break;
        // case 7:
        // sys_kill(tf->x[0]);
            // break;
    }
}
