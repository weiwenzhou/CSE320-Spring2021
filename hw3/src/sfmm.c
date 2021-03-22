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
        block = (sf_block *) sf_check_free_list(actual, place, 16);
        if (block != NULL)
            break;
    }
    while (block == NULL) {
        if (sf_increase_wilderness()) {
            sf_errno = ENOMEM;
            return NULL;
        }
        block = (sf_block *) sf_check_free_list(actual, NUM_FREE_LISTS-1, 16);
    }
    return &block->body;
}

void sf_free(void *pp) {
    if (sf_check_pointer(pp)) 
        abort();
    sf_block *block = (sf_block *) ((char *) pp - 8);
    sf_add_to_free_list(block);
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    if (rsize == 0) {
        sf_free(pp);
        return NULL;
    }
    if (sf_check_pointer(pp))
        abort();
    sf_header size = (*((sf_header *) (pp-8))) & ~0x3;
    // calculate the number of bytes: min(header (8) + size + padding to be 16 byte align, 32)
    size_t actual = 8 + rsize;
    actual = (actual % 16 == 0) ? actual:((actual/16 + 1) * 16);
    actual = (actual < 32) ? 32:actual;
    if (actual <= size) {
        return &((sf_block *) sf_allocate_block(pp-8, actual))->body;
    }
    void *block = sf_malloc(rsize);
    if (block == NULL)
        return NULL;
    memcpy(block, pp, size-8);
    sf_free(pp);
    return block;
}

void *sf_memalign(size_t size, size_t align) {
    sf_block *start = (sf_block *) sf_mem_start();
    sf_block *end = (sf_block *) sf_mem_end();
    if (start == end) {
        sf_initialize_heap();
    }
    if (size == 0)
        return NULL;
    // check align is at least 32 and a power of 2
    if (align < 32 || ((align & (align-1)) != 0)) {
        sf_errno = EINVAL;
        return NULL;
    }
    // calculate the number of bytes: header (8) + size + padding to be 'align' byte align
    size_t actual = 8 + size;
    actual = (actual % align == 0) ? actual:((actual/align + 1) * align);
    // calculate the class size
    int class = 0;
    while (class < NUM_FREE_LISTS-1) {
        if (actual <= (32 << class))
            break;
        class++;
    }
    sf_block *block;
    for (int place = class; place < NUM_FREE_LISTS; place++) {
        block = (sf_block *) sf_check_free_list(actual, place, align);
        if (block != NULL)
            break;
    }
    while (block == NULL) {
        if (sf_increase_wilderness()) {
            sf_errno = ENOMEM;
            return NULL;
        }
        block = (sf_block *) sf_check_free_list(actual, NUM_FREE_LISTS-1, align);
    }
    return &block->body;
}
