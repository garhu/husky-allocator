#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct free_list_cell {
    size_t size;
    int arena_id;
    struct free_list_cell* next;
} free_list_cell;

typedef struct block_header {
    size_t size;
    int arena_id;
} block_header;

typedef struct arena {
    pthread_mutex_t lock;
    struct free_list_cell* free_list;
    bool used;
} arena;

void* xmalloc(size_t bytes);
void  xfree(void* ptr);
void* xrealloc(void* prev, size_t bytes);

#endif
