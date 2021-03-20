#include "sfmm.h"

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