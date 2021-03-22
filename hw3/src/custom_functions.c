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

void *sf_check_free_list(size_t size, int index, size_t align) {
    // check if there is space in free_list
    sf_block *list = &sf_free_list_heads[index];
    if (list->body.links.next == list) // empty
        return NULL;
    // search through the linked list
    sf_block *current = list->body.links.next;
    do {
        size_t padding = (((size_t) current+8) % align);
        if (size+padding <= (current->header & ~0x3)) {
            current->body.links.prev->body.links.next = current->body.links.next;
            current->body.links.next->body.links.prev = current->body.links.prev;
            return sf_allocate_block(current, size);
        }

        current = current->body.links.next;
    } while (current != list);
    
    return NULL;
}

void sf_add_to_free_list(sf_block *block) {
    // coalesce before adding a block to a list
    sf_header size = block->header & ~0x3;
    sf_block *next = (sf_block *) ((char *) block + size);
    if ((next->header & THIS_BLOCK_ALLOCATED) == 0) {
        next->body.links.prev->body.links.next = next->body.links.next;
        next->body.links.next->body.links.prev = next->body.links.prev;
        size += next->header & ~0x3;
    } else
        next->header &= ~PREV_BLOCK_ALLOCATED;
    if ((block->header & PREV_BLOCK_ALLOCATED) == 0) { // prev_block not allocated
        sf_header *prev_size = (sf_header *) (((char *) block) - 8);
        block = (sf_block *) (((char *) block) - (*prev_size & ~0x3));
        block->header += size;
        block->body.links.prev->body.links.next = block->body.links.next;
        block->body.links.next->body.links.prev = block->body.links.prev;
    } else 
        block->header = size | (block->header & PREV_BLOCK_ALLOCATED);
    *((sf_header *) (((char *) block) + (block->header & ~(0x3)) - 8)) = block->header;
    // get block size
    size = block->header & ~0x3;
    // last block is always place in the wilderness at index 7 
    int class = (((char *) block + size) == sf_mem_end() -8)? 7:0; 
    while (class < NUM_FREE_LISTS-1) {
        if (size <= (32 << class))
            break;
        class++;
    }
    block->body.links.next = sf_free_list_heads[class].body.links.next;
    block->body.links.prev = &sf_free_list_heads[class];
    sf_free_list_heads[class].body.links.next->body.links.prev = block;
    sf_free_list_heads[class].body.links.next = block;
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

int sf_check_pointer(void *pp) {
    if (pp == NULL)
        return -1;
    if (((size_t) pp) % 16 != 0)
        return -1; 
    if (pp < sf_mem_start()) // not inside the heap
        return -1;
    sf_block *block = (sf_block *) ((char *) pp - 8);
    sf_header size = block->header & ~0x3;
    if (size % 16 != 0)
        return -1;
    if (size < 32)
        return -1;
    if ((block->header & THIS_BLOCK_ALLOCATED) == 0)
        return -1;
    if (*((sf_header *) (((char *) block) + (block->header & ~(0x3)) - 8)) == block->header) 
        return -1;
    if ((char *) block + size > (char *) (sf_mem_end()-8)) 
        return -1;
    if ((block->header & PREV_BLOCK_ALLOCATED) == 0) { // check if prev block is not allocated
        sf_header *prev_header = (sf_header *) (((char *) block) - 8);
        if ((*prev_header & THIS_BLOCK_ALLOCATED) != 0)
            return -1;
    }
    return 0;
}

void *sf_allocate_block(void *block, size_t size) {
    sf_block *current = (sf_block *) block;
    sf_header length = current->header & ~0x3;
    // decide whether to split or not
    if (length - size < 32) { // don't split
        current->header = current->header | THIS_BLOCK_ALLOCATED;
        ((sf_block *)((char *) current + size))->header |= PREV_BLOCK_ALLOCATED;
    } else { // split
        sf_block *new_block = (sf_block *) (((char *) current) + size);
        current->header = size | THIS_BLOCK_ALLOCATED | (current->header & PREV_BLOCK_ALLOCATED);

        new_block->header = (length - size) | PREV_BLOCK_ALLOCATED;
        *((sf_header *) (((char *) new_block) + (new_block->header & ~(0x3)) - 8)) = new_block->header;
        sf_add_to_free_list(new_block);
    }
    return current;
}