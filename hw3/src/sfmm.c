/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "custom_functions.h"

void *sf_malloc(size_t size) {
    sf_block *start = (sf_block *) sf_mem_start();
    sf_block *end = (sf_block *) sf_mem_end();
    if (start == end) {
        sf_initialize_heap();
    }
    // calculate the number of bytes: min(header (8) + size + padding to be 16 byte align, 32)
    // check if 8+size is a multiple of 16
    // if true then use actual
    // else take the (quotient+1)*16 which should be enough bytes to hold actual
    // then check if actual is at least 32 bytes (minimum block)
    return NULL;
}

void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
