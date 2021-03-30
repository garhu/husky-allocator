
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#

#include "xmalloc.h"

#define ARENAS 5

const size_t PAGE_SIZE = 4096;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static arena arenas[ARENAS];
__thread int arena_id = -1;

void
check_rv(int rv)
{
    if (rv == -1) {
        perror("oops");
        fflush(stdout);
        fflush(stderr);
        abort();
    }
}

long
free_list_length()
{
    return 0;
}

/*
static
size_t
div_up(size_t xx, size_t yy)
{
    // This is useful to calculate # of pages
    // for large allocations.
    size_t zz = xx / yy;

    if (zz * yy == xx) {
        return zz;
    }
    else {
        return zz + 1;
    }
}
*/

void
free_list_remove(int bucket_index)
{
    free_list_node* curr = arenas[arena_id].buckets[bucket_index]->free_list;

    // if the second node in the free list exists,
    // set the head of the list to be the second node
    if (!curr->next) {
        arenas[arena_id].buckets[bucket_index]->free_list = curr->next;
        arenas[arena_id].buckets[bucket_index]->num_cells--;
    }
    else {
        arenas[arena_id].buckets[bucket_index]->free_list = 0;
        arenas[arena_id].buckets[bucket_index]->num_cells = 0;
    }
}

/*
void
coalesce(free_list_cell* arg1, free_list_cell* arg2)
{
    
}
*/

void
free_list_add(void* addr, size_t size, int item_arena)
{
    pthread_mutex_lock(&arenas[item_arena].lock);
    
}

int
get_bucket_index(size_t size)
{
    for (int ii = 0; ii < 8; ++ii) {
        if (1 << (ii + 4) >= size) {
            return ii;
        }   
    }
    return -1;
}

void*
available_addr(int bucket_index)
{
    free_list_node* curr = arenas[arena_id].buckets[bucket_index]->free_list;
    
    if (curr) {
        return (void*)curr;
    }

    return 0;
}

void
init_buckets(int arena)
{
    for (int ii = 0; ii < 8; ++ii) {
        arenas[arena].buckets[ii] = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, 
                                         MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        printf("%p\n", &arenas[arena].buckets[ii]);
        ((bucket*)arenas[arena].buckets[ii])->size = (size_t)(1 << (ii + 4));
        ((bucket*)arenas[arena].buckets[ii])->num_cells = 0;
        ((bucket*)arenas[arena].buckets[ii])->free_list = 0;
        printf("%p\n", &arenas[arena].buckets[ii]->free_list);
    }
}

void
init_arena()
{
    for(int ii = 0; ii < ARENAS; ++ii) {
        if (!arenas[ii].used) {
            arena_id = ii;
            pthread_mutex_init(&arenas[ii].lock, 0);
            init_buckets(ii);
            arenas[ii].used = 1;
            break;
        }
    }
}

void
fill_bucket(int bucket_index)
{
    size_t size = PAGE_SIZE - sizeof(bucket);
    size_t bucket_size = arenas[arena_id].buckets[bucket_index]->size;
    int num_cells = size / bucket_size;
    
    void* free_list_addr = (void*)arenas[arena_id].buckets[bucket_index]->free_list;
    printf("free_list_addr = %p\n", &free_list_addr);
    void* next_free_list_addr = free_list_addr;

    printf("%d\n", num_cells);    
    for (int ii = 0; ii < num_cells; ++ii) {
        printf("filling%d\n", ii);
        ((free_list_node*)free_list_addr)->arena_id = arena_id;
        next_free_list_addr = (void*)((uintptr_t)free_list_addr + bucket_size);
        ((free_list_node*)free_list_addr)->next = (free_list_node*)next_free_list_addr;
        free_list_addr = next_free_list_addr;
    }
}

void*
xmalloc(size_t size)
{
    if (arena_id == -1) {
        pthread_mutex_lock(&lock);
        init_arena();
        pthread_mutex_unlock(&lock);
    }
    
    size += sizeof(block_header);

    if (size <= 2048) {
        int bucket_index = get_bucket_index(size);

        if (arenas[arena_id].buckets[bucket_index]->num_cells == 0) {
            fill_bucket(bucket_index);
        }
        
        // get the address of the first block in the bucket
        // void* addr = available_addr(bucket_index);

        // remove the first block in the bucket
        pthread_mutex_lock(&arenas[arena_id].lock);
        free_list_remove(bucket_index);
        pthread_mutex_unlock(&arenas[arena_id].lock);
    }


    return 0;
}

void
xfree(void* item)
{

}

void*
xrealloc(void* prev, size_t bytes)
{
    pthread_mutex_lock(&lock);
    xfree(prev);    
   
    pthread_mutex_unlock(&lock);
    return xmalloc(bytes);
}





