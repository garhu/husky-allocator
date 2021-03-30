#ifndef HMALLOC_H
#define HMALLOC_H

// Husky Malloc Interface
// cs3650 Starter Code

// free_list_cell is based off of Professor Tuck's 
// implementation in lecture
typedef struct free_list_cell {
    size_t size;
    struct free_list_cell* next;
} free_list_cell;

// header is based off of Professor Wilson's
// implementation in his slides
typedef struct header {
    size_t size;
} header;

#endif
