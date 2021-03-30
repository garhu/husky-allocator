
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "xmalloc.h"

const size_t PAGE_SIZE = 4096;
static free_list_cell* free_list;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
    int count = 0;
    free_list_cell* temp = free_list;
    
    while (temp) {
        count++;
        temp = temp->next;
    }

    return count;
}

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

void
free_list_remove(void* addr)
{
    free_list_cell* curr = free_list;
    free_list_cell* prev = 0;

    while (curr) {
        if ((uintptr_t)curr == (uintptr_t)addr) {
            // if curr is the only cell in free_list
            if (!prev && !curr->next) {
                free_list = 0;
            } 
            // if curr is the first cell in free_list
            else if (!prev) {
                free_list = curr->next;
            }
            // if curr is the last cell in free_list
            else if (!curr->next) {
                prev->next = 0;
            }
            // if curr is in the middle of free_list
            else {
                prev->next = curr->next;
            }
        }

        prev = curr;
        curr = curr->next;
    }
}

void
coalesce(free_list_cell* arg1, free_list_cell* arg2)
{
    // coalesce if the address + size of arg1 is equal to the address of arg2
    if ((uintptr_t)arg1 + arg1->size == (uintptr_t)arg2) {
        arg1->size += arg2->size;
        arg1->next = arg2->next;
    }
}

void
free_list_add(void* addr, size_t size)
{
    free_list_cell* curr = free_list;

    // if free_list is empty, set free_list to be a new free_list_cell*
    if (!curr) {
        free_list = (free_list_cell*)addr;
        free_list->size = size; 
        free_list->next = 0;
        return;
    }

    free_list_cell* prev = 0;
    
    // iterate until the address is less than the 
    // current cell's address in the sorted list
    while (curr) {
        if ((uintptr_t)addr < (uintptr_t)curr) {
            // if there is a cell before curr
            if (prev) {
               free_list_cell* new_cell = (free_list_cell*)addr;
               new_cell->size = size;
               new_cell->next = curr;
               prev->next = new_cell;

               coalesce(new_cell, curr);
               coalesce(prev, new_cell);
            }
            // if curr is the first element
            else {
                free_list = (free_list_cell*)addr;
                free_list->size = size;
                free_list->next = curr;

                coalesce(free_list, curr);
            }
        
            break;
        }

        // if the address is greater than 
        // all cells' addresses in the list
        if (!curr->next) {
            free_list_cell* new_cell = (free_list_cell*)addr;
            new_cell->size = size;
            new_cell->next = 0;
            curr->next = new_cell;

            coalesce(curr, new_cell);

            break;
        }

        prev = curr;
        curr = curr->next;
    }
}

void*
available_addr(size_t size)
{
    free_list_cell* curr = free_list;
    while (curr) {
        if (curr->size >= size) {
            return (void*)curr;
        }
        curr = curr->next;
    }
    return 0;
}

void*
xmalloc(size_t size)
{
    pthread_mutex_lock(&lock);
    size += sizeof(size_t);
    void* addr;
    
    // requests with B < 1 page
    if (size < PAGE_SIZE) {
        size_t block_size;
        
        // find the first big enough block
        // on the free_list, if available
        addr = available_addr(size);
       
        // if a big enough block on the free_list exists,
        // select it and remove from free_list
        if (addr) {
            block_size = ((free_list_cell*)addr)->size;
            free_list_remove(addr);
        }
        // else mmap a new block
        else {
            addr = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            check_rv(*((int*)addr));
            
            block_size = PAGE_SIZE;
        }

        // if the leftover is big enough to store
        // a free_list_cell, return the extra to the free_list
        if (block_size - size >= sizeof(free_list_cell)) {
            // free_addr is the address of the free_list_cell
            void* free_addr = (void*)(((uintptr_t)addr) + size);
            // free_size is the size ofthe free_list_cell
            size_t free_size = block_size - size; 
            free_list_add(free_addr, free_size); 
        }  

        block_header* head = addr;
        head->size = size;
    }
    // requests with B >= 1 page
    else {
        // calculate number of pages needed for this block
        size_t num_pages = div_up(size, PAGE_SIZE);
        size = num_pages * PAGE_SIZE;

        // allocate that many pages
        addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        check_rv(*((int*)addr));

        // fill in the size of the block as (# of pages * 4096)
        block_header* head = addr;
        head->size = size;
    }

    pthread_mutex_unlock(&lock);

    // return pointer to the block after size field
    return (void*)((size_t*)addr + 1);
}

void
xfree(void* item)
{
    pthread_mutex_lock(&lock);
    
    // item_addr is the address of the item including its size
    void* item_addr = (void*)((size_t*)item - 1);
    // size is the size of the item
    size_t size = ((block_header*)item_addr)->size;

    // if block is < 1 page then stick it on free_list
    if (size < PAGE_SIZE) {
        // add to the free list
        free_list_add(item_addr, size);
    }
    // if block is >= 1 page then munmap it
    else {
        munmap(item_addr, size);
    }
    
    pthread_mutex_unlock(&lock);
}

void*
xrealloc(void* prev, size_t bytes)
{
    pthread_mutex_lock(&lock);
    

    pthread_mutex_unlock(&lock);
    return xmalloc(bytes);
}





