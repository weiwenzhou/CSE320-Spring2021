#include "sfmm.h"

/**
 * Initializes the empty heap. Adds the new free block to the wilderness.
 * 
 */
void sf_initialize_heap();

/**
 * Searches for a block of size in the free list at index. If found allocate that 
 * block and split it. The lower part is use for allocation. The upper part is 
 * put into the appropriate block. Do not split if the upper part is less than 
 * 32 bytes (4 rows).
 * 
 * @param size The number of bytes to be allocated
 * @param index The index of the free list to check
 * 
 * @return Returns NULL if block is not found, otherwise a pointer to the allocated block
 */
void *sf_check_free_list(size_t size, int index);

/**
 * Adds the given block pointer to appropriate free list class size.
 * 
 * @param block Address of memory 
 */
void sf_add_to_free_list(sf_block *block);