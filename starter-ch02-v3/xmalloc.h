#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct free_list_node {
    int arena_id;
    struct free_list_node* next;
} free_list_node;

typedef struct block_header {
    size_t size;
    int arena_id;
} block_header;

typedef struct bucket {
    size_t size;
    int num_cells;
    free_list_node* free_list;
} bucket;

typedef struct arena {
    pthread_mutex_t lock;
    bucket* buckets[8];
    bool used;
} arena;

void* xmalloc(size_t bytes);
void  xfree(void* ptr);
void* xrealloc(void* prev, size_t bytes);

#endif
