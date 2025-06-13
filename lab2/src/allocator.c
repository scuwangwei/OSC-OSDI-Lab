#include <stddef.h>  // for size_t
#include "allocator.h"
#include "mini_uart.h"


/*
in linker script I reserve 64KB for heap
this should keep runing,because it manages heap area
*/

// linker symbol
extern char heap_start;
extern char heap_end;

//declare a static heap ptr,so that every program will know the current heap_ptr,and heap_ptr is a ptr put a symbol address
static char *heap_ptr = 0;


void* simple_allocator(size_t size)
{
    if (!heap_ptr) {
        heap_ptr = &heap_start;  // initialize on first use
    }

    //record ptr for return
    char *prev_ptr = heap_ptr;

    // align to 8 bytes
    size = (size + 7) & ~7;

    if (heap_ptr + size > &heap_end) {
        mini_uart_send_string("Heap out of memory\r\n");
        return 0;  // out of memory
    }

    //update heap_ptr address
    heap_ptr += size;
    return (void*)prev_ptr;
}

/*
for testing,keep allocating memory until out of memory
*/
void test_allocator_exhaustion() {
    while (1) {
        void *ptr = simple_allocator(64); // try allocating 64 bytes at a time
        if (ptr == 0) {
            mini_uart_send_string("Out of memory!\r\n");
            break;
        } else {
            mini_uart_send_string("Allocated block ");
            mini_uart_send_string(" at address: 0x");
            mini_uart_send_hex((unsigned int)ptr);
            mini_uart_send_string("\r\n");
        }
    }
}