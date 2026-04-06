#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h> 

#define PAGE_SIZE 4096 // 4KB per page
#define HEAP_ORDER_MAX 12 //Heap Max Order
#define HEAP_SIZE (PAGE_SIZE << HEAP_ORDER_MAX) //4MB
#define PAGE_NUM (HEAP_SIZE / PAGE_SIZE) //Page Number
#define ALLOCATED 2
#define NOT_ALLOCATED 1
#define BELONG_LARGER_BLOCK 0
#define POOL_NUM 8
#define POOL_ID_MAX 0xFFFFFFFF

typedef struct block {
    int order;//record order
    int status; // 0 = belong to larger block, 1 = free, 2 = allocated
    struct block *next;//next free block
} block_t;

typedef struct chunk {
    struct chunk *next;
} chunk_t;

typedef struct {
    unsigned int pool_id;
}__attribute__((aligned(8))) chunk_header_t;//align 8 bytes

typedef struct {
    unsigned int chunk_total;
    unsigned int chunk_size;
    chunk_t *free_list;
    void *start_addr;
} pool_t;
void buddy_init();
void *malloc(unsigned long size);
void free(void *addr);
void print_free_list();
void pool_init();

// void* simple_allocator(size_t size);
// void test_allocator_exhaustion();

#endif
