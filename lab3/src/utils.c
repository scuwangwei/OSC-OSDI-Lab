#include "utils.h"
#include "mini_uart.h"

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

//parse char to int
int parse2int(const char *buffer, unsigned int *value)
{
    unsigned int result = 0;
    for (int i = 0; buffer[i] != '\0'; ++i) {
        if (buffer[i] >= '0' && buffer[i] <= '9') {
            result = result * 10 + (buffer[i] - '0');
        } else {
            return -1;  // error
        }
    }
    *value = result;
    return 0;  // sucess
}

// int to str
char* int2char(int value) 
{
    static char str[21];
    char buffer[21];
    int i = 0;

    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    if (value < 0) {
        value = -value;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    // Reverse copy
    for (int j = 0; j < i; j++) {
        str[j] = buffer[i - j - 1];
    }
    str[i] = '\0';

    return str;
}

void simple_strncpy(char *dest, const char *src, int max_len)
{
    int i = 0;
    for (; i < max_len - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

int parse_msg_secs(char *buffer,char *msg, int msg_size, unsigned int *delay)
{
    char *p = buffer;
    char *last_space = NULL;
    while(*p)
    {
        if(*p == ' ') last_space = p;
        p++;
    }
    if(last_space == NULL || last_space == buffer) return -1;
    if(parse2int((last_space + 1),delay) < 0) return -1;

    while(buffer != last_space)
    {
        *msg = *buffer;
        msg++;
        buffer++;
    }
    *msg = '\0';

    return 0;
}