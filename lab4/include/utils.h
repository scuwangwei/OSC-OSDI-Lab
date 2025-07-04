#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h> 

#define STACK_CAPACITY 128
#define ALIGN8(x) (((x) + 7) & ~7)

typedef struct {
    void *data[STACK_CAPACITY];
    int top;
} stack_t;

void stack_init(stack_t *s);
int  stack_is_empty(stack_t *s);
int  stack_is_full(stack_t *s);
int  stack_push(stack_t *s, void *item);
void *stack_pop(stack_t *s);
void *stack_peek(stack_t *s);

int strcmp(const char *s1, const char *s2,const int size);
int strings_compare(const char *s1, const char *s2);
int hex2int(const char *str, int len);
const char* align(const char* ptr, size_t n);
int parse2int(const char *buffer, unsigned int *value);
char* int2char(int value) ;
void simple_strncpy(char *dest, const char *src, int max_len);
int parse_msg_secs(char *buffer,char *msg, int msg_size, unsigned int *delay);

static inline uint32_t be32_to_cpu(uint32_t val) {
    return __builtin_bswap32(val);
}



#endif