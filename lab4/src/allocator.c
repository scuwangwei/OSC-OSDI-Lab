#include <stddef.h>
#include "allocator.h"
#include "mini_uart.h"
#include "utils.h"


/*
in linker script I reserve 4MB for heap
this should keep runing,because it manages heap area
*/

// linker symbol
extern char heap_start;
extern char heap_end;

/*----------------Buddy System--------------------*/

static block_t page_array[PAGE_NUM];
static block_t *free_list[HEAP_ORDER_MAX + 1];

//input physical address and switch it to page index in page arrary
static inline int page_idx(void *addr) 
{
    return ((char *)addr - (char *)&heap_start) / PAGE_SIZE;
}

//input page array index and switch it to physical address
static inline void *idx_to_addr(int idx) 
{
    return (void *)((char *)&heap_start + idx * PAGE_SIZE);
}

//input block address and switch it to page array index
static inline int get_block_idx(block_t *block) {
    return block - page_array;
}

//get its buddy block address
static block_t *get_buddy(block_t *block) {

    if (block->order >= HEAP_ORDER_MAX) {
        return NULL;//it has no buddy
    }

    int idx = get_block_idx(block);
    int order = block->order;
    int buddy_idx = idx ^ (1 << order);
    return &page_array[buddy_idx];
}

//init buddy system
void buddy_init()
{
    page_array[0].order = HEAP_ORDER_MAX;
    page_array[0].status = NOT_ALLOCATED;
    page_array[0].next = NULL;

    for(int i=1;i<PAGE_NUM;i++)
    {
        page_array[i].order = -1;
        page_array[i].status = BELONG_LARGER_BLOCK;
        page_array[i].next = 0;
    }

    free_list[HEAP_ORDER_MAX] = &page_array[0];

    for(int i=1;i<HEAP_ORDER_MAX;i++)
    {
        free_list[i] = NULL;
    }
}

//get minimal order number that satisfy size n
int get_min_order(int n) {
    int k = 0;
    n--;

    while (n > 0) {
        n >>= 1;
        k++;
    }
    return k;
}

//remove a block from a free list
void remove_free_list(block_t *block)
{
    int order = block->order;
    block_t *curr = free_list[order];
    block_t *prev = NULL;

    while (curr != NULL) {
        if (curr == block) {
            // case: removing head of list
            if (prev == NULL) {
                free_list[order] = curr->next;
            } else {
                prev->next = curr->next;
            }
            block->next = NULL;
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    mini_uart_send_string("remove_free_list: block not found!\r\n");
}

//free a block in buddy system
void free_block(block_t* block)
{
    //change status to NOT_ALLOCATED
    block->status = NOT_ALLOCATED;
    //if the block is max order,return
    if(block->order == HEAP_ORDER_MAX) return;



    //combine if both of those two are free recursivly
    block_t *buddy;
    while(block->order != HEAP_ORDER_MAX)
    {

        buddy = get_buddy(block);
        if(buddy->status == NOT_ALLOCATED)
        {

            remove_free_list(buddy);

            //determine which address to be the new larger block's start address
            if(block < buddy)
            {
                buddy->order = -1;
                buddy->status = BELONG_LARGER_BLOCK;
                block->order = ++(block->order);
                block->status = NOT_ALLOCATED;
            }
            else
            {
                buddy->order = ++(block->order);
                buddy->status = NOT_ALLOCATED;
                block->order = -1;
                block->status = BELONG_LARGER_BLOCK;

                block = buddy;
            }
            
        }

        else break;
    }

    //add to free list
    block->next = free_list[block->order];
    free_list[block->order] = block;

    
}

//split a block into 2 blocks
void split_block(block_t *block, block_t **left, block_t **right)
{
    int order = block->order;
    if (order == 0 || order > HEAP_ORDER_MAX) return;

    int idx = get_block_idx(block);
    int half_size = 1 << (order - 1);

    *left = &page_array[idx];
    *right = &page_array[idx + half_size];

    (*left)->order = order - 1;
    (*left)->status = NOT_ALLOCATED;
    (*left)->next = NULL;

    (*right)->order = order - 1;
    (*right)->status = NOT_ALLOCATED;
    (*right)->next = free_list[order - 1];

    free_list[order - 1] = *right;
}

//allocate a block in buddy system
void *alloc_block(int size)
{
    //calculate required pages and min order number that can meet the requirement
    int required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int min_order = get_min_order(required_pages);
    if(min_order > HEAP_ORDER_MAX)
    {
        mini_uart_send_string("Too large,unable to allocate\r\n");
        return 0;
    }
    //keep finding the free list which have free block
    int target_order = min_order;
    while(free_list[target_order] == NULL && target_order <= HEAP_ORDER_MAX)
    {
        target_order++;
    }

    //no free list meet the requirement
    if(target_order > HEAP_ORDER_MAX)
    {
        mini_uart_send_string("No free spaces left\r\n");
        return 0;
    }
    //allocate the free block,and remove the block from free list
    block_t *block = free_list[target_order];
    block->next = NULL;
    free_list[target_order] = free_list[target_order]->next;

    block_t *left;
    block_t *right;
    //split block if needed
    while(target_order > min_order)
    {
        

        split_block(block,&left,&right);
        block = left;
        target_order--;
    }

    //allocate the block and return the physical address
    block->status = ALLOCATED;
    void *addr = idx_to_addr(get_block_idx(block));
    mini_uart_send_string("Allocate address at: ");
    mini_uart_send_hex((unsigned int)addr);
    mini_uart_send_string("\r\n");
    mini_uart_send_string("Allocated Block Order: ");
    mini_uart_send_string(int2char(target_order));
    mini_uart_send_string("\r\n");
    return addr;


}

//print each order current free list in buddy system
void print_free_list()
{
    for(int i=HEAP_ORDER_MAX;i>=0;i--)
    {
        block_t *cur = free_list[i];
        mini_uart_send_string("Order ");
        mini_uart_send_string(int2char(i));
        mini_uart_send_string(" free list :");
        while(cur)
        {
            int idx = cur - page_array;
            mini_uart_send_string("page array idx [");
            mini_uart_send_string(int2char(idx));
            mini_uart_send_string("]-->");
            cur = cur->next;
        }
        mini_uart_send_string("NULL\r\n");
    }
}

/*---------------------Memory Pool------------------------------*/
const int pool_size_class[POOL_NUM] = {16, 32, 64, 128, 256, 512, 1024, 2048};
pool_t pools[POOL_NUM];

//init pool
void pool_init() {
    for (int i = 0; i < POOL_NUM; i++) {
        pools[i].chunk_size = pool_size_class[i];
        pools[i].free_list = NULL;
    }
}

//allocate a contigous memory from pools
void *pool_alloc(int size) {
    mini_uart_send_string("Checking if the pools can allocate a chunks for memory allocation\r\n");
    //find a pool size that can be allocated with minimal size
    for (int i = 0; i < POOL_NUM; i++) {
        if (size <= pools[i].chunk_size) {
            mini_uart_send_string("Find a pool,pool size: \r\n");
            mini_uart_send_string(int2char(pool_size_class[i]));
            mini_uart_send_string("\r\n");
            pool_t *pool = &pools[i];
            //if no free list in this pool,then allocate a page for it
            if (!pool->free_list) {
                mini_uart_send_string("No free list in pool,will allocate a page from buddy system\r\n");
                char *page = (char *)alloc_block(PAGE_SIZE);
                if (!page) return NULL;
                //split the page into the chunks and add them to free list
                int chunk_total = PAGE_SIZE / pools[i].chunk_size;
                pools[i].chunk_total = chunk_total;
                pools[i].start_addr = (void *)page;
                for (int j = 0; j < chunk_total; j++) {
                    char *chunk_base = page + j * pools[i].chunk_size;
                    chunk_t *chunk = (chunk_t *)chunk_base;
                    chunk->next = pool->free_list;
                    pool->free_list = chunk;
                }
            }

            //allocate a chunk
            chunk_t *raw_chunk = pool->free_list;
            pool->free_list = raw_chunk->next;
            (pools[i].chunk_total)--;

            //cast it to a chunk header and assign a pool id
            chunk_header_t *header = (chunk_header_t *)raw_chunk;
            header->pool_id = i;
            mini_uart_send_string("Allocate a chunk from pool\r\n");
            mini_uart_send_string("Pool ID: ");
            mini_uart_send_string(int2char(i));
            mini_uart_send_string("\r\n");
            //return the chunk address( +1 offset)
            return (void *)(header + 1);
        }
    }
    //no pool satisfied
    return 0;
}

//free a contiguous memory from pool
void pool_free(chunk_header_t *header) {

    //find the pool id by header
    int pool_id = header->pool_id;
    //cast it to chunk_t and add the chunk back to the pool free list
    chunk_t *chunk = (chunk_t *)header;
    chunk->next = pools[pool_id].free_list;
    pools[pool_id].free_list = chunk;
    (pools[pool_id].chunk_total)++;
    mini_uart_send_string("free the block with pool id: ");
    mini_uart_send_string(int2char(pool_id));
    mini_uart_send_string("\r\n");

    //check the chunk total,if all chunks in the pool are free,then free the page back to buddy system
    int chunk_total = PAGE_SIZE / pools[pool_id].chunk_size;
    if(chunk_total == pools[pool_id].chunk_total) 
    {
        mini_uart_send_string("All chunks in this pool are free,free the pool page back to buddy system\r\n");
        mini_uart_send_string("The start page for this pool address is at ");
        mini_uart_send_hex((unsigned int)pools[pool_id].start_addr);
        mini_uart_send_string("\r\n");
        int idx = page_idx(pools[pool_id].start_addr);
        free_block(&page_array[idx]);
        pools[pool_id].free_list = NULL;
        pools[pool_id].chunk_total = 0;
        pools[pool_id].start_addr = NULL;
    }
}



/*---------------Dynamic Memory Allocator-----------------------*/
void *malloc(unsigned long size) 
{
    void *addr;
    //allocate memory from pool
    if(addr = pool_alloc(size))
    {
        return addr;
    }
    //allocate memory from buddy system
    else
    {
        mini_uart_send_string("No pool satisfied, Allocate pages in buddy system for memory allocation\r\n");
        // Fallback for large allocations
        chunk_header_t *header = (chunk_header_t *)alloc_block(size + sizeof(chunk_header_t));
        if (!header) return NULL;
        header->pool_id = POOL_ID_MAX;
        addr = (void *)(header + 1);
        mini_uart_send_string("data start at:");
        mini_uart_send_hex((unsigned int)addr);
        mini_uart_send_string("\r\n");
        return addr;
    }

}

void free(void *addr) 
{
    if (!addr) return;
    chunk_header_t *header = ((chunk_header_t *)addr) - 1;
    //free memory from buddy system
    if (header->pool_id == POOL_ID_MAX) {
        int idx = page_idx((void *)header);
        if (idx < 0 || idx >= PAGE_NUM) 
        {
            mini_uart_send_string("Illegal Access\r\n");
            return;
        }

        block_t *block = &page_array[idx];
        if (block->status != ALLOCATED) 
        {
            mini_uart_send_string("Illegal Free\r\n");
            return;
        }
        mini_uart_send_string("free the block,start idx in page array at ");
        mini_uart_send_string(int2char(get_block_idx(block)));
        mini_uart_send_string("\r\nAnd the address is ");
        mini_uart_send_hex((unsigned int)header);
        free_block(block);
        return;
    }
    //free buddy from pool
    else
    {
        mini_uart_send_string("This block belongs to a pool,free the block and put it back to pool\r\n");
        pool_free(header);
    }
    
}

// /*---------------Simple Allocator-----------------*/
//declare a static heap ptr,so that every program will know the current heap_ptr,and heap_ptr is a ptr put a symbol address
//static char *heap_ptr = 0;
// void* simple_allocator(size_t size)
// {
//     if (!heap_ptr) {
//         heap_ptr = &heap_start;  // initialize on first use
//     }

//     //record ptr for return
//     char *prev_ptr = heap_ptr;

//     // align to 8 bytes
//     size = (size + 7) & ~7;

//     if (heap_ptr + size > &heap_end) {
//         mini_uart_send_string("Heap out of memory\r\n");
//         return 0;  // out of memory
//     }

//     //update heap_ptr address
//     heap_ptr += size;
//     return (void*)prev_ptr;
// }

// /*
// for testing,keep allocating memory until out of memory
// */
// void test_allocator_exhaustion() {
//     while (1) {
//         void *ptr = simple_allocator(64); // try allocating 64 bytes at a time
//         if (ptr == 0) {
//             mini_uart_send_string("Out of memory!\r\n");
//             break;
//         } else {
//             mini_uart_send_string("Allocated block ");
//             mini_uart_send_string(" at address: 0x");
//             mini_uart_send_hex((unsigned int)ptr);
//             mini_uart_send_string("\r\n");
//         }
//     }
// }