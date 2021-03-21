#include "sfmm.h"
#include "custom_functions.h"

void sf_initialize_heap() {
    // initialize free list
    for (int i = 0; i < NUM_FREE_LISTS; i++) {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }

    // adds a new page of memory
    (sf_block *) sf_mem_grow();

    sf_block *prologue = (sf_block *) (sf_mem_start() + 8);
    sf_block *epilogue = (sf_block *) (sf_mem_end()-8);
    sf_block *free_block = (sf_block *) (sf_mem_start() + 8 + 32);
    prologue->header = 32 + THIS_BLOCK_ALLOCATED;
    epilogue->header = 0 + THIS_BLOCK_ALLOCATED;
    free_block->header = PAGE_SZ - 48 + PREV_BLOCK_ALLOCATED;
    *((sf_header *) (((char *) free_block) + (free_block->header & ~(0x3)) - 8)) = free_block->header;


    sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = free_block;
    sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = free_block;
    free_block->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS-1];
    free_block->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS-1];
}

void *sf_check_free_list(size_t size, int index) {
    // check if there is space in free_list
    sf_block *list = &sf_free_list_heads[index];
    if (list->body.links.next == list) // empty
        return NULL;
    // search through the linked list
    sf_block *current = list->body.links.next;
    do {
        if (size <= (current->header & ~0x3)) {
            sf_header length = current->header & ~0x3;
            // decide whether to split or not
            if (length - size < 32) { // don't split
                current->body.links.prev->body.links.next = current->body.links.next;
                current->body.links.next->body.links.prev = current->body.links.prev;
                current->header = current->header | THIS_BLOCK_ALLOCATED;
            } else { // split
                sf_block *new_block = (sf_block *) (((char *) current) + size);
                current->body.links.prev->body.links.next = current->body.links.next;
                current->body.links.next->body.links.prev = current->body.links.prev;
                current->header = size | THIS_BLOCK_ALLOCATED | (current->header & PREV_BLOCK_ALLOCATED);

                new_block->header = (length - size) | PREV_BLOCK_ALLOCATED;
                sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = new_block;
                sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = new_block;
                new_block->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS-1];
                new_block->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS-1];
                 *((sf_header *) (((char *) new_block) + (new_block->header & ~(0x3)) - 8)) = new_block->header;
            }
            return current;
        }

        current = current->body.links.next;
    } while (current != list);
    
    return NULL;
}