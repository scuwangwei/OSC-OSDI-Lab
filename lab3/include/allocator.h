#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h> 


void* simple_allocator(size_t size);
void test_allocator_exhaustion();

#endif
