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
    return sf_generalize_allocation(size, 16);
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
    if (sf_check_pointer(pp)) {
        sf_errno = EINVAL;
        return NULL;
    }
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
    // check align is at least 32 and a power of 2
    if (align < 32 || ((align & (align-1)) != 0)) {
        sf_errno = EINVAL;
        return NULL;
    }
    return sf_generalize_allocation(size, align);
}
