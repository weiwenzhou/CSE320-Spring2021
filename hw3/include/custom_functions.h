#include "sfmm.h"

/**
 * Initializes the empty heap. Adds the new free block to the wilderness.
 * 
 */
void sf_initialize_heap();

/**
 * Searches for a block of size in the free list at index. If found allocate that 
 * block with proper alignment and split it. The lower part is use for allocation. 
 * The upper part is put into the appropriate block. Do not split if the upper part 
 * is less than 32 bytes (4 rows).
 * 
 * @param size The number of bytes to be allocated
 * @param index The index of the free list to check
 * @param align The address alignment boundary
 * 
 * @return Returns NULL if block is not found, otherwise a pointer to the allocated block
 */
void *sf_check_free_list(size_t size, int index, size_t align);

/**
 * Adds the given block pointer to appropriate free list class size.
 * 
 * @param block Address of memory 
 */
void sf_add_to_free_list(sf_block *block);

/**
 * Increases the size of the wilderness by one page of memory (8192 bytes).
 * 
 * @return Returns -1 if error, otherwise 0. 
 */
int sf_increase_wilderness();

/**
 * Checks if the pointer is valid
 * 
 * @return Return -1 if invalid, otherwise 0.
 */
int sf_check_pointer(void *pp);

/**
 * Allocates the size of bytes in the memory address block. Assumes that
 * block is a valid pointer in the heaper and size can fit inside of the block. 
 * 
 * @param block Address of memory
 * @param size The number of bytes to allocate
 * 
 * @return The address of the newly allocated block.
 */
void *sf_allocate_block(void *block, size_t size);

/**
 * Allocates a block of memory with a specific alignment (similar to 
 * sf_memalign with the minimum alignment being 16 instead of 32). Alignment
 * is assume to be a power of 2. A helper function for sf_malloc and sf_memalign. 
 * 
 * @param size The number of bytes requested to be allocated.
 * @param align The alignment required of the returned pointer.
 * 
 * @return If size is 0, then NULL is returned without setting sf_errno.
 * If size is nonzero, then if the allocation is successful a pointer to
 * a valid region of memory of requested size with the requested alignment
 * is returned. If the allocation is not successful then the NULL is returned
 * and sf_errno is set to ENOMEM. 
 */
void *sf_generalize_allocation(size_t size, size_t align);