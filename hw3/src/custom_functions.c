#include "custom_functions.h"
#include "debug.h"
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

    sf_add_to_free_list(free_block);
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
                *((sf_header *) (((char *) new_block) + (new_block->header & ~(0x3)) - 8)) = new_block->header;
                sf_add_to_free_list(new_block);
            }
            // if current block is the last block and modifies epilogue
            if ((char *) current + size == sf_mem_end() - 8) {
                ((sf_block *)(sf_mem_end() - 8))->header |= PREV_BLOCK_ALLOCATED;
            }
            return current;
        }

        current = current->body.links.next;
    } while (current != list);
    
    return NULL;
}

void sf_add_to_free_list(sf_block *block) {
    // get block size
    sf_header size = block->header & ~0x3;
    // last block is always place in the wilderness at index 7 
    int class = (((char *) block + size) == sf_mem_end() -8)? 7:0; 
    while (class < NUM_FREE_LISTS-1) {
        if (size <= (32 << class))
            break;
        class++;
    }
    block->body.links.prev = sf_free_list_heads[class].body.links.prev;
    block->body.links.next = &sf_free_list_heads[class];
    sf_free_list_heads[class].body.links.prev->body.links.next = block;
    sf_free_list_heads[class].body.links.prev = block;
}

int sf_increase_wilderness() {
    sf_block *old_epilogue = (sf_block *) (sf_mem_end()-8);
    sf_block *page = (sf_block *) sf_mem_grow();
    if (page == NULL) {
        return -1;
    }
    sf_block *new_epilogue = (sf_block *) (sf_mem_end()-8);
    new_epilogue->header = old_epilogue->header;
    // if prev of old epilogue is free than combine
    if ((old_epilogue->header & PREV_BLOCK_ALLOCATED) == 0) {
        // get header 
        sf_header *prev_size = (sf_header *) (((char *) old_epilogue) - 8);
        sf_block *prev_block = (sf_block *) (((char *) old_epilogue) - (*prev_size & ~0x3));
        prev_block->header = prev_block->header + PAGE_SZ;
        *((sf_header *) (((char *) prev_block) + (prev_block->header & ~(0x3)) - 8)) = prev_block->header;
    } else {
        old_epilogue->header = PAGE_SZ | PREV_BLOCK_ALLOCATED;
        *((sf_header *) (((char *) old_epilogue) + (old_epilogue->header & ~(0x3)) - 8)) = old_epilogue->header;
        sf_add_to_free_list(old_epilogue);
    }
    return 0;
}