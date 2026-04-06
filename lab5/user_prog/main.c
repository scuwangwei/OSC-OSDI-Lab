#include <stdint.h>
#include <stddef.h>

/*
Utility Functions
*/
void int2char(int num, char *buf)
{
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    int start = 0, end = i - 1;
    while (start < end) {
        char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        start++;
        end--;
    }
}

/*
System Call Function for User Program
*/

int getpid()
{
    int pid;

    asm volatile(
        "mov x8, #0\n"   // syscall number = 0 (getpid)
        "svc 0\n"        // trigger exception
        "mov %0, x0\n"   // return value in x0
        : "=r"(pid)
        :
        : "x0", "x8"
    );

    return pid;
}

size_t uart_read(char buf[], size_t size)
{
    size_t ret;

    asm volatile(
        "mov x0, %1\n"   // buf
        "mov x1, %2\n"   // size
        "mov x8, #3\n"   // syscall number
        "svc 0\n"
        "mov %0, x0\n"
        : "=r"(ret)
        : "r"(buf), "r"(size)
        : "x0", "x1", "x8"
    );

    return ret;
}

size_t uart_write(const char buf[], size_t size)
{
    size_t ret;

    asm volatile(
        "mov x0, %1\n"   // buf
        "mov x1, %2\n"   // size
        "mov x8, #2\n"   // syscall number
        "svc 0\n"
        "mov %0, x0\n"
        : "=r"(ret)
        : "r"(buf), "r"(size)
        : "x0", "x1", "x8"
    );

    return ret;
}

int exec(const char* name, char *const argv[])
{
    int ret;

    asm volatile(
        "mov x0, %1\n"   // program name
        "mov x1, %2\n"   // argv
        "mov x8, #1\n"
        "svc 0\n"
        "mov %0, x0\n"
        : "=r"(ret)
        : "r"(name), "r"(argv)
        : "x0", "x1", "x8"
    );

    return ret;
}

int fork()
{
    int ret;

    asm volatile(
        "mov x8, #4\n"
        "svc 0\n"
        "mov %0, x0\n"
        : "=r"(ret)
        :
        : "x0", "x8"
    );

    return ret;
}

void exit()
{
    asm volatile(
        "mov x8, #5\n"
        "svc 0\n"
        :
        :
        : "x8"
    );

}

int mbox_call(unsigned char ch, unsigned int *mbox)
{
    int ret;

    asm volatile(
        "mov x0, %1\n"   // channel
        "mov x1, %2\n"   // mbox buffer
        "mov x8, #6\n"
        "svc 0\n"
        "mov %0, x0\n"
        : "=r"(ret)
        : "r"(ch), "r"(mbox)
        : "x0", "x1", "x8"
    );

    return ret;
}

void kill(int pid)
{
    asm volatile(
        "mov x0, %0\n"
        "mov x8, #7\n"
        "svc 0\n"
        :
        : "r"(pid)
        : "x0", "x8"
    );
}


int main()
{

    const char *msg1 = "Hello from user program!\r\n";

    char buf[32];
    int pid = getpid();
    int2char(pid,buf);
    uart_write(msg1, 26);
    const char *msg2 = "Current pid: \r\n";
    uart_write(msg2, 15);
    uart_write(buf, 1);
    uart_write("\r\n", 2);


    while(1) {}
}
//Add the section attribute so the linker finds it
void _start() __attribute__((section("._start")));

void _start()
{
    main();
    
    // 3. Tell the kernel this process is done
    // exit(); 
    
    // Failsafe: trap the CPU in case the kernel's exit syscall fails

}