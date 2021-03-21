/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"
#include "custom_functions.h"

void *sf_malloc(size_t size) {
    sf_block *start = (sf_block *) sf_mem_start();
    sf_block *end = (sf_block *) sf_mem_end();
    if (start == end) {
        sf_initialize_heap();
    }
    if (size == 0)
        return NULL;
    // calculate the number of bytes: min(header (8) + size + padding to be 16 byte align, 32)
    size_t actual = 8 + size;
    actual = (actual % 16 == 0) ? actual:((actual/16 + 1) * 16);
    actual = (actual < 32) ? 32:actual;
    // calculate the class size
    int class = 0;
    while (class < NUM_FREE_LISTS-1) {
        if (actual <= (32 << class))
            break;
        class++;
    }
    sf_block *block;
    for (int place = class; place < NUM_FREE_LISTS; place++) {
        block = (sf_block *) sf_check_free_list(actual, place);
        if (block != NULL)
            break;
    }
    while (block == NULL) {
        if (sf_increase_wilderness()) {
            sf_errno = ENOMEM;
            return NULL;
        }
        block = (sf_block *) sf_check_free_list(actual, NUM_FREE_LISTS-1);
    }
    return &block->body;
}

void sf_free(void *pp) {
    if (sf_check_pointer(pp)) 
        abort();
    sf_block *block = (sf_block *) ((char *) pp - 8);
    sf_header size = block->header & ~0x3;
    sf_block *next = (sf_block *) ((char *) block + size);
    if ((next->header & THIS_BLOCK_ALLOCATED) == 0) {
        next->body.links.prev->body.links.next = next->body.links.next;
        next->body.links.next->body.links.prev = next->body.links.prev;
        size += next->header & ~0x3;
    } else
        next->header ^= PREV_BLOCK_ALLOCATED;
    if ((block->header & PREV_BLOCK_ALLOCATED) == 0) { // prev_block not allocated
        sf_header *prev_size = (sf_header *) (((char *) block) - 8);
        block = (sf_block *) (((char *) block) - (*prev_size & ~0x3));
        block->header += size;
        block->body.links.prev->body.links.next = block->body.links.next;
        block->body.links.next->body.links.prev = block->body.links.prev;
    } else 
        block->header = size | (block->header & PREV_BLOCK_ALLOCATED);
    *((sf_header *) (((char *) block) + (block->header & ~(0x3)) - 8)) = block->header;
    sf_add_to_free_list(block);
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
