#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>

typedef struct free_list_cell {
    size_t size;
    struct free_list_cell* next;
} free_list_cell;

typedef struct block_header {
    size_t size;
} block_header;

void* xmalloc(size_t bytes);
void  xfree(void* ptr);
void* xrealloc(void* prev, size_t bytes);

#endif
