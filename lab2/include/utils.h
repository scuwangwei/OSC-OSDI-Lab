#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h> 

int strcmp(const char *s1, const char *s2,const int size);
int strings_compare(const char *s1, const char *s2);
int hex2int(const char *str, int len);
const char* align(const char* ptr, size_t n);

static inline uint32_t be32_to_cpu(uint32_t val) {
    return __builtin_bswap32(val);
}



#endif