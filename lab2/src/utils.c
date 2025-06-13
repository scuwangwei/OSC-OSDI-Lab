#include "utils.h"

/*
This strcmp need a comapre size
return value
    0 - same
    1 - different
*/
int strcmp(const char *s1, const char *s2,const int size)
{
    for(int i=0;i<size;i++)
    {
        if(s1[i] != s2[i]) return 1;
    }
    return 0;
}

/*return value
    0 : same
    positive int : different and s1 len > s2 len
    negative int : different and s1 len < s2 len    
*/
int strings_compare(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2; // return difference if not equal
        }
        s1++;
        s2++;
    }
    return *s1 - *s2; // handles cases where one string is longer
}

/*convert hex to int*/
int hex2int(const char *str, int len) {
    int val = 0;
    for (int i = 0; i < len; i++) {
        val <<= 4;
        if (str[i] >= '0' && str[i] <= '9') val |= str[i] - '0';
        else if (str[i] >= 'A' && str[i] <= 'F') val |= str[i] - 'A' + 10;
        else if (str[i] >= 'a' && str[i] <= 'f') val |= str[i] - 'a' + 10;
    }
    return val;
}

const char* align(const char* ptr, size_t n) {
    uintptr_t p = (uintptr_t)ptr;
    p = (p + (n - 1)) & ~(n - 1);
    return (const char*)p;
}