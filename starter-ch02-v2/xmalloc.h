#ifndef HMALLOC_H
#define HMALLOC_H

#include <stddef.h>

// Husky Malloc Interface
// cs3650 Starter Code

typedef struct hm_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} hm_stats;

// free_list_cell is based off of Professor Tuck's 
// implementation in lecture
typedef struct free_list_cell {
    size_t size;
    struct free_list_cell* next;
} free_list_cell;

// header is based off of Professor Wilson's
// implementation in his slides
typedef struct size_header {
    size_t size;
} size_header;

hm_stats* hgetstats();
void hprintstats();

void* xmalloc(size_t size);
void xfree(void* item);
void* xrealloc(void* ptr, size_t size);

#endif
