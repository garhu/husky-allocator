#ifndef HMALLOC_H
#define HMALLOC_H

#include <stddef.h>
#include <stdbool.h>

// Husky Malloc Interface
// cs3650 Starter Code

typedef struct hm_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} hm_stats;

typedef struct spacer {
    int a;
    short b; 
    bool c;
} spacer;

// free_list_cell is based off of Professor Tuck's 
// implementation in lecture
typedef struct free_list_cell {
    bool used;
    size_t size;
    struct free_list_cell* prev;
    struct free_list_cell* next;
} free_list_cell;

typedef struct block_header {
    bool used;
    size_t size;
    // spacer s;
} header_t;

typedef struct block_footer {
    size_t size;
} block_footer;

hm_stats* hgetstats();
void hprintstats();

void* xmalloc(size_t size);
void xfree(void* item);
void* xrealloc(void* ptr, size_t size);

#endif
