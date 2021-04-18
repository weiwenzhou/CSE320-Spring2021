#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"
#define TEST_TIMEOUT 15
#define max(x, y) (((x) > (y)) ? (x) : (y))

static bool free_list_is_empty()
{
    for (int i = 0; i < NUM_FREE_LISTS; i++) {
       if(sf_free_list_heads[i].body.links.next != &sf_free_list_heads[i] ||
        sf_free_list_heads[i].body.links.prev != &sf_free_list_heads[i])
           return false;
    }
    return true;
}

/*
 * This function determines the index of the freelist appropriate for a
 * specified block size.
 */
static int free_list_index(size_t size)
{
    size_t s = 32;
    int i = 0;
    while(i < NUM_FREE_LISTS-1) {
        if(size <= s)
            return i;
        i++;
	// Power-of-two sequence
	s *= 2;
    }
    // If we reach here, the block must be in the second-to-last free list,
    // which has arbitrarily large blocks.
    // The last list has only the wilderness block.
    return NUM_FREE_LISTS-2;
}

static bool block_is_in_free_list(sf_block * abp)
{
    sf_block *bp = NULL;
    size_t block_size = abp->header & ~((1LU << 4) - 1);
    int i = free_list_index(block_size);

    sf_block *next_block = ((sf_block *)((char *)(abp) + block_size));
    if (next_block == ((sf_block *)(sf_mem_end() - sizeof(sf_header))))
        i = NUM_FREE_LISTS - 1; // wilderness block

    bp = (&sf_free_list_heads[i])->body.links.next;
    while(bp != &sf_free_list_heads[i]) {
        if (bp == abp)  return true;
        bp = bp->body.links.next;
    }
    return false;
}


void _assert_free_list_is_empty()
{
    cr_assert(free_list_is_empty(), "Free list is not empty");
}

/*
 * Function checks if all blocks are unallocated in free_list.
 * This function does not check if the block belongs in the specified free_list.
 */
void _assert_free_list_is_valid()
{
    for (int i = 0; i < NUM_FREE_LISTS; i++) {
        sf_block *bp = sf_free_list_heads[i].body.links.next;
        int limit = 10000;
        while (bp != &sf_free_list_heads[i] && limit--)
        {
            cr_assert(((bp->header & 0x1) == 0),
                "Block %p in free list is marked allocated", &(bp)->header);
            bp = bp->body.links.next;
        }
        cr_assert(limit != 0, "Bad free list");
    }
}

/**
 * Checks if a block follows documentation constraints
 *
 * @param bp pointer to the block to check
 */
void _assert_block_is_valid(sf_block *bp)
{

    // proper block alignment & size
    size_t aligned_payload = ((((size_t)((bp)->body.payload) + (1<<4) - 1) >>4) <<4);
    cr_assert(((bp)->body.payload) == (void*)aligned_payload,
        "Block %p is not properly aligned", bp);

    size_t block_size = bp->header & ~((1LU << 4) - 1);

    cr_assert((block_size >= 32 && block_size <= 100 * PAGE_SZ && ((block_size) & 0b1111) == 0),
        "Block size is invalid for %p. Got: %lu",
        bp, block_size);

    sf_block *next_block = ((sf_block *)((char *)(bp) + block_size));
    // prev alloc bit of next block == alloc bit of this block
    cr_assert((char *)next_block==(char*)(sf_mem_end() - sizeof(sf_header)) // next block == epilogue
    || ((next_block->header & 0x2) != 0) == ((bp->header & 0x1) != 0), // prev of next block & curr block both marked as allocated
        "Prev allocated bit is not correctly set for %p. Should be: %d",
        &(next_block)->header, (bp->header & 0x1) != 0);


    // other issues to check
    sf_footer *footer = ((sf_footer *)((char *)(next_block) - sizeof(sf_footer)));
    cr_assert((bp->header & 0x1) != 0 || bp->header == *footer, // footer is valid
        "Block's footer does not match header for %p", &(bp)->header);


    if ((bp->header & 0x1) != 0) {
    cr_assert(!block_is_in_free_list(bp),
        "Allocated block at %p is also in the free list", &(bp)->header);
    }
    else {
    cr_assert(block_is_in_free_list(bp),
      "Free block at %p is not contained in the free list", &(bp)->header);
    }
}


void _assert_heap_is_valid(void)
{
    sf_block *bp;
    int limit = 10000;
    if (sf_mem_start() == sf_mem_end())
        cr_assert(free_list_is_empty(),
            "The heap is empty, but the free list is not");

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    sf_block *prologue = ((sf_block *)(align_start - sizeof(sf_header)));
    sf_block *epilogue = sf_mem_end() - sizeof(sf_header);
    char *first_block = ((char *)prologue + 32); // first block

    bp = (sf_block *)first_block;
    while (limit-- && bp < epilogue) {
        _assert_block_is_valid(bp);
        bp = (sf_block *)((char *)bp + (bp->header & ~((1LU << 4) - 1)));
    }
    _assert_free_list_is_valid();

}

/**
 * Asserts a block's info.
 *
 * @param bp pointer to the beginning of the block.
 * @param alloc The expected allocation bit for the block.
 * @param b_size The expected block size.
 */
void _assert_block_info(sf_block * bp, int alloc, size_t b_size)
{
    debug("SIZE :  %ld\n",b_size);

    cr_assert(((((bp)->header) & 0x1) != 0) == alloc,
        "Block %p has wrong allocation status (got %d, expected %d)",
        &(bp)->header, ((((bp)->header) & 0x1) != 0), alloc);
    cr_assert((bp->header & ~((1LU << 4) - 1)) == b_size,
        "Block %p has wrong block_size (got %lu, expected %lu)",
        &(bp)->header, (bp->header & ~((1LU << 4) - 1)), b_size);
}

/**
 * Asserts payload pointer is not null.
 *
 * @param pp payload pointer.
 */
void _assert_nonnull_payload_pointer(void *pp)
{
    cr_assert(pp != NULL, "Payload pointer should not be NULL");
}

/**
 * Asserts payload pointer is null.
 *
 * @param pp payload pointer.
 */
void _assert_null_payload_pointer(void *pp)
{
    cr_assert(pp == NULL, "Payload pointer should be NULL");
}

/**
 * Assert the total number of free blocks of a specified size.
 * If size == 0, then assert the total number of all free blocks.
 *
 * @param size the size of free blocks to count.
 * @param count the expected number of free blocks to be counted.
 */
void _assert_free_block_count(size_t size, int count)
{
    int cnt = 0;

    for (int i = 0; i < NUM_FREE_LISTS; i++) {
        sf_block * bp = sf_free_list_heads[i].body.links.next;
        while (bp != &sf_free_list_heads[i]) {
            debug("block in free list %p: size = %zu\n", bp, (bp->header & ~((1LU << 4) - 1)));
            if (size == 0 || size == (bp->header & ~((1LU << 4) - 1)))
                cnt++;
            bp = bp->body.links.next;
        }
    }
    if(size)
        cr_assert_eq(cnt, count, "Wrong number of free blocks of size %ld (exp=%d, found=%d)",
           size, count, cnt);
    else
        cr_assert_eq(cnt, count, "Wrong number of free blocks (exp=%d, found=%d)",
           count, cnt);
}


/**
 * Assert the sf_errno.
 *
 * @param n the errno expected
 */
void _assert_errno_eq(int n)
{
    cr_assert_eq(sf_errno, n, "sf_errno has incorrect value (value=%d, exp=%d)", sf_errno, n);
}

//Not all remainders of blocks are passed on to the next
//if it's less than MIN_BLOCK_SIZE, it's thrown away
//-> cannot simply count by division as previously done
int get_malloc_count(size_t each_block_size)
{
    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t start_point = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header); // prologue + epilogue size
    size_t end_point = PAGE_SZ;
    size_t diff = 0;
    size_t remainder = 0;
    size_t remainder_sum = 0;
    int cnt = 0;
    while(end_point<=16*PAGE_SZ)
    {
        diff = end_point - start_point;
        cnt += diff / each_block_size;
        remainder = diff % each_block_size;
        remainder_sum += remainder;
        // debug("%ld\n",remainder);
        //if remainder is smaller than 32 (min block size), make it zero
        if(remainder < 32)
            remainder =  0;
        start_point = end_point - remainder;
        end_point += PAGE_SZ;

    }
    // debug("COUNT : %d\n\n",*cnt);
    // debug("REMAINDER : %ld\n\n",*remainder);

    return cnt;
}

/*==================================TEST CASES======================================*/

/*
 * Do one malloc and check that the prologue and epilogue are correctly initialized
 */
Test(sf_memsuite_grading, initialization, .timeout = TEST_TIMEOUT)
{
	size_t sz = 1;
	void* p  = sf_malloc(sz);
	cr_assert(p!=NULL, "The pointer should NOT be null after a malloc");
	_assert_heap_is_valid();
}

/*
 * Single malloc tests, up to the size that forces a non-minimum block size.
 */
Test(sf_memsuite_grading, single_malloc_1, .timeout = TEST_TIMEOUT)
{
	size_t sz = 1;
	void * x = sf_malloc(sz);

	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_sz = PAGE_SZ - unused_area_size - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);
	_assert_free_block_count(exp_free_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, single_malloc_16, .timeout = TEST_TIMEOUT)
{
	size_t sz = 16;
	void * x = sf_malloc(sz);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_sz = PAGE_SZ - unused_area_size - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);
	_assert_free_block_count(exp_free_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, single_malloc_32, .timeout = TEST_TIMEOUT)
{
	size_t sz = 32;
	void * x = sf_malloc(sz);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_sz = PAGE_SZ - unused_area_size - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);
	_assert_free_block_count(exp_free_sz, 1);

	_assert_errno_eq(0);
}


/*
 * Single malloc test, of a size exactly equal to what is left after initialization.
 * Requesting the exact remaining size (leaving space for the header)
 */
Test(sf_memsuite_grading, single_malloc_exactly_one_page_needed, .timeout = TEST_TIMEOUT)
{
    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t sz = PAGE_SZ - sizeof(sf_header) - unused_area_size;
	void * x = sf_malloc(sz);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

	_assert_free_block_count(0, 0);

	_assert_errno_eq(0);
}

/*
 * Single malloc test, of a size just larger than what is left after initialization.
 */
Test(sf_memsuite_grading, single_malloc_more_than_one_page_needed, .timeout = TEST_TIMEOUT)
{
	size_t sz = PAGE_SZ - sizeof(sf_header) - sizeof(sf_header);
	void * x = sf_malloc(sz);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_sz = PAGE_SZ * 2 - unused_area_size - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);
	_assert_free_block_count(exp_free_sz, 1);

	_assert_errno_eq(0);
}

/*
 * Single malloc test, of multiple pages.
 */
Test(sf_memsuite_grading, single_malloc_three_pages_needed, .timeout = TEST_TIMEOUT)
{
	size_t sz = (size_t)((int)(PAGE_SZ * 3 / 1000) * 1000);
	void * x = sf_malloc(sz);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_sz = PAGE_SZ * 3 - unused_area_size - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);
	_assert_free_block_count(exp_free_sz, 1);

	_assert_errno_eq(0);
}

/*
 * Single malloc test, unsatisfiable.
 * There should be one single large block.
 */
Test(sf_memsuite_grading, single_malloc_max, .timeout = TEST_TIMEOUT)
{
	size_t sz = 16 * PAGE_SZ;
	void * x = sf_malloc(sz);
	_assert_null_payload_pointer(x);
	_assert_heap_is_valid();

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_sz = 16 * PAGE_SZ - unused_area_size;
	_assert_free_block_count(exp_free_sz, 1);

	_assert_errno_eq(ENOMEM);
}

/*
 * Malloc/free with/without coalescing.
 */
Test(sf_memsuite_grading, malloc_free_no_coalesce, .timeout = TEST_TIMEOUT)
{
	size_t sz1 = 200;
	size_t sz2 = 300;
	size_t sz3 = 400;

	void * x = sf_malloc(sz1);
	_assert_nonnull_payload_pointer(x);

	void * y = sf_malloc(sz2);
	_assert_nonnull_payload_pointer(y);

	void * z = sf_malloc(sz3);
	_assert_nonnull_payload_pointer(z);

	sf_free(y);


	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
     0, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_block_info(((sf_block *)((char *)(z) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

	_assert_free_block_count(max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32), 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_2_sz = PAGE_SZ
    - max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;

	_assert_free_block_count(exp_free_2_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, malloc_free_coalesce_lower, .timeout = TEST_TIMEOUT)
{
	size_t sz1 = 200;
	size_t sz2 = 300;
	size_t sz3 = 400;
	size_t sz4 = 500;

	void * x = sf_malloc(sz1);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * y = sf_malloc(sz2);
	_assert_nonnull_payload_pointer(y);
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * z = sf_malloc(sz3);
	_assert_nonnull_payload_pointer(z);
	_assert_block_info(((sf_block *)((char *)(z) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * w = sf_malloc(sz4);
	_assert_nonnull_payload_pointer(w);
	_assert_block_info(((sf_block *)((char *)(w) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	sf_free(y);
	sf_free(z);

	size_t sz = max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
     + max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);

	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
     0, sz);
	_assert_block_info(((sf_block *)((char *)(w) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

	_assert_free_block_count(max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32), 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_2_sz = PAGE_SZ
	- max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
	- unused_area_size;
	_assert_free_block_count(exp_free_2_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, malloc_free_coalesce_upper, .timeout = TEST_TIMEOUT)
{
	size_t sz1 = 200;
	size_t sz2 = 300;
	size_t sz3 = 400;
	size_t sz4 = 500;

	void * x = sf_malloc(sz1);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * y = sf_malloc(sz2);
	_assert_nonnull_payload_pointer(y);
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * z = sf_malloc(sz3);
	_assert_nonnull_payload_pointer(z);
	_assert_block_info(((sf_block *)((char *)(z) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * w = sf_malloc(sz4);
	_assert_nonnull_payload_pointer(w);
	_assert_block_info(((sf_block *)((char *)(w) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	sf_free(z);
	sf_free(y);
	size_t sz = max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);

	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    0, sz);
	_assert_block_info(((sf_block *)((char *)(w) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

	_assert_free_block_count(max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32), 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_2_sz = PAGE_SZ
    - max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
	- unused_area_size;
	_assert_free_block_count(exp_free_2_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, malloc_free_coalesce_both, .timeout = TEST_TIMEOUT)
{
	size_t sz1 = 200;
	size_t sz2 = 300;
	size_t sz3 = 400;
	size_t sz4 = 500;

	void * x = sf_malloc(sz1);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * y = sf_malloc(sz2);
	_assert_nonnull_payload_pointer(y);
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * z = sf_malloc(sz3);
	_assert_nonnull_payload_pointer(z);
	_assert_block_info(((sf_block *)((char *)(z) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * w = sf_malloc(sz4);
	_assert_nonnull_payload_pointer(w);
	_assert_block_info(((sf_block *)((char *)(w) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	sf_free(x);
	sf_free(z);
	sf_free(y);
	size_t sz = max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);

	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))), 0, sz);
	_assert_heap_is_valid();

	_assert_free_block_count(max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    + max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32), 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_2_sz = PAGE_SZ
    - max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz3+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz4+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
	- unused_area_size;
	_assert_free_block_count(exp_free_2_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, malloc_free_first_block, .timeout = TEST_TIMEOUT)
{
	size_t sz1 = 200;
	size_t sz2 = 300;

	void * x = sf_malloc(sz1);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * y = sf_malloc(sz2);
	_assert_nonnull_payload_pointer(y);
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
     1, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	sf_free(x);

	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
     0, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
	_assert_heap_is_valid();

	_assert_free_block_count(max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32), 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t exp_free_2_sz = PAGE_SZ
    - max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
	- unused_area_size;
	_assert_free_block_count(exp_free_2_sz, 1);

	_assert_errno_eq(0);
}

Test(sf_memsuite_grading, malloc_free_last_block, .timeout = TEST_TIMEOUT)
{
    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t sz1 = 200;
	size_t sz2 = PAGE_SZ - max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
	- unused_area_size - (1 << 4);

	void * x = sf_malloc(sz1);
	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	void * y = sf_malloc(sz2);
	_assert_nonnull_payload_pointer(y);
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz2+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	sf_free(y);

	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	size_t exp_free_sz = PAGE_SZ
    - max(((((size_t)(sz1+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;
	_assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
     0, exp_free_sz);
	_assert_free_block_count(exp_free_sz, 1);

	_assert_heap_is_valid();

	_assert_errno_eq(0);
}

/*
 * Check that malloc leaves no splinter.
 */
Test(sf_memsuite_grading, malloc_with_splinter, .timeout = TEST_TIMEOUT)
{
    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

	size_t sz = PAGE_SZ - unused_area_size - 32;
	void * x = sf_malloc(sz);

	_assert_nonnull_payload_pointer(x);
	_assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32) + (1 << 4));
	_assert_heap_is_valid();

	_assert_free_block_count(0, 0);

	_assert_errno_eq(0);
}

/*
 *  Allocate small blocks until memory exhausted.
 */
Test(sf_memsuite_grading, malloc_to_exhaustion, .timeout = TEST_TIMEOUT)
{
	size_t sz = 100;
	int exp_mallocation = get_malloc_count(max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

	int limit = 0;
	int exp_limit = abs(limit - exp_mallocation);

	void * x;
	while ((x = sf_malloc(sz)) != NULL && limit++!=exp_limit)
	{
		sf_block * bp = ((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload)));
		size_t size = (bp->header & ~((1LU << 4) - 1));
        // Not all blocks will be the same size due to splitting restrictions.
		cr_assert(size == max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32) ||
			size == (max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32) + (1 << 4)),
			"block has incorrect size (size=%lu, exp=%lu or %lu)",
			size, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32),
			max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32) + (1 << 4));
	}

	// fprintf(stderr, "limit: %d, exp_limit: %d, exp_mallocation: %d\n",
	// 	limit, exp_limit, exp_mallocation);
	cr_assert_eq(limit, exp_limit, "Memory not exhausted when it should be");

	_assert_heap_is_valid();

	_assert_errno_eq(ENOMEM);
}

/*
 *  Test sf_memalign handling invalid arguments:
 *  If align is not a power of two or is less than the minimum block size,
 *  then NULL is returned and sf_errno is set to EINVAL.
 *  If size is 0, then NULL is returned without setting sf_errno.
 */
Test(sf_memsuite_grading, sf_memalign_test_1, .timeout = TEST_TIMEOUT)
{
    /* Test size is 0, then NULL is returned without setting sf_errno */
    int old_errno = sf_errno;  // Save the current errno just in case
    sf_errno = ENOTTY;  // Set errno to something it will never be set to as a test (in this case "not a typewriter")
    size_t arg_align = 32;
    size_t arg_size = 0;
    void *actual_ptr = sf_memalign(arg_size, arg_align);
    cr_assert_null(actual_ptr, "sf_memalign didn't return NULL when passed a size of 0");
    _assert_errno_eq(ENOTTY);  // Assert that the errno didn't change
    sf_errno = old_errno;  // Restore the old errno

    /* Test align less than the minimum block size */
    arg_align = 1U << 2;  // A power of 2 that is still less than MIN_BLOCK_SIZE
    arg_size = 25;  // Arbitrary
    actual_ptr = sf_memalign(arg_size, arg_align);
    cr_assert_null(actual_ptr, "sf_memalign didn't return NULL when passed align that was less than the minimum block size");
    _assert_errno_eq(EINVAL);

    /* Test align that isn't a power of 2 */
    arg_align = (32 << 1) - 1;  // Greater than MIN_BLOCK_SIZE, but not a power of 2
    arg_size = 65;  // Arbitrary
    actual_ptr = sf_memalign(arg_size, arg_align);
    cr_assert_null(actual_ptr, "sf_memalign didn't return NULL when passed align that wasn't a power of 2");
    _assert_errno_eq(EINVAL);
}

/*
Test that memalign returns an aligned address - using minimum block size for alignment
 */
Test(sf_memsuite_grading, sf_memalign_test_2, .timeout = TEST_TIMEOUT)
{
    size_t arg_align = 1U << 6; // Use the minimum block size for alignment
    size_t arg_size = 25;  // Arbitrary
    void *x = sf_memalign(arg_size, arg_align);
    _assert_nonnull_payload_pointer(x);
    if (((unsigned long)x & 0x3F) != 0) {  // Test if any of the lower-order 6 bits are 1 - if so, then the address is not aligned
        cr_assert(1 == 0, "sf_memalign didn't return an aligned address!");
    }
}

/*
Test that memalign returns aligned, usable memory
 */
Test(sf_memsuite_grading, sf_memalign_test_3, .timeout = TEST_TIMEOUT)
{
    size_t arg_align = 1U << 7; // Use larger than minimum block size for alignment
    size_t arg_size = 129;  // Arbitrary
    void *x = sf_memalign(arg_size, arg_align);
    _assert_nonnull_payload_pointer(x);
    if (((unsigned long)x & 0x7F) != 0) {  // Test if any of the lower-order 6 bits are 1 - if so, then the address is not aligned
        cr_assert(1 == 0, "sf_memalign didn't return an aligned address!");
    }
    _assert_heap_is_valid();
}

/*
 * Check LIFO discipline on free list
 */
Test(sf_memsuite_grading, malloc_free_lifo, .timeout=TEST_TIMEOUT)
{
    size_t sz = 200;
    void * x = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * u = sf_malloc(sz);
    _assert_nonnull_payload_pointer(u);
    _assert_block_info(((sf_block *)((char *)(u) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * y = sf_malloc(sz);
    _assert_nonnull_payload_pointer(y);
    _assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * v = sf_malloc(sz);
    _assert_nonnull_payload_pointer(v);
    _assert_block_info(((sf_block *)((char *)(v) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * z = sf_malloc(sz);
    _assert_nonnull_payload_pointer(z);
    _assert_block_info(((sf_block *)((char *)(z) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * w = sf_malloc(sz);
    _assert_nonnull_payload_pointer(w);
    _assert_block_info(((sf_block *)((char *)(w) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    sf_free(x);
    sf_free(y);
    sf_free(z);

    void * z1 = sf_malloc(sz);
    _assert_nonnull_payload_pointer(z1);
    _assert_block_info(((sf_block *)((char *)(z1) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * y1 = sf_malloc(sz);
    _assert_nonnull_payload_pointer(y1);
    _assert_block_info(((sf_block *)((char *)(y1) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));
    void * x1 = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x1);
    _assert_block_info(((sf_block *)((char *)(x1) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    cr_assert(x == x1 && y == y1 && z == z1,
      "malloc/free does not follow LIFO discipline");

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

    size_t exp_free_block_size = PAGE_SZ
    - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32) * 6
    - unused_area_size;
    _assert_free_block_count(exp_free_block_size, 1);

    _assert_errno_eq(0);
}

/*
 * Realloc tests.
 */
Test(sf_memsuite_grading, realloc_larger, .timeout=TEST_TIMEOUT)
{
    size_t sz = 200;
    size_t nsz = 1024;

    void * x = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    void * y = sf_realloc(x, nsz);
    _assert_nonnull_payload_pointer(y);
    _assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    _assert_free_block_count(max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32), 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

    size_t exp_sz = PAGE_SZ
    - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;
    _assert_free_block_count(exp_sz, 1);

    _assert_errno_eq(0);
}

Test(sf_memsuite_grading, realloc_smaller, .timeout=TEST_TIMEOUT)
{
    size_t sz = 1024;
    size_t nsz = 200;

    void * x = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    void * y = sf_realloc(x, nsz);
    _assert_nonnull_payload_pointer(y);
    _assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    cr_assert_eq(x, y, "realloc to smaller size did not return same payload pointer");

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

    size_t exp_sz = PAGE_SZ
    - max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;
    _assert_free_block_count(exp_sz, 1);
    _assert_errno_eq(0);
}

Test(sf_memsuite_grading, realloc_same, .timeout=TEST_TIMEOUT)
{
    size_t sz = 1024;
    size_t nsz = 1024;

    void * x = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    void * y = sf_realloc(x, nsz);
    _assert_nonnull_payload_pointer(y);
    _assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    cr_assert_eq(x, y, "realloc to same size did not return same payload pointer");

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

    size_t exp_sz = PAGE_SZ
    - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;//EPILOGUE_SIZE;
    _assert_free_block_count(exp_sz, 1);

    _assert_errno_eq(0);
}

Test(sf_memsuite_grading, realloc_splinter, .timeout=TEST_TIMEOUT)
{
    size_t sz = 1024;
    size_t nsz = 1020;

    void * x = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    void * y = sf_realloc(x, nsz);
    _assert_nonnull_payload_pointer(y);
    _assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    cr_assert_eq(x, y, "realloc to smaller size did not return same payload pointer");

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

    size_t exp_sz = PAGE_SZ
    - max(((((size_t)(nsz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;
    _assert_free_block_count(exp_sz, 1);
    _assert_errno_eq(0);
}

Test(sf_memsuite_grading, realloc_size_0, .timeout=TEST_TIMEOUT)
{
    size_t sz = 1024;
    void * x = sf_malloc(sz);
    _assert_nonnull_payload_pointer(x);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    void * y = sf_malloc(sz);
    _assert_nonnull_payload_pointer(y);
    _assert_block_info(((sf_block *)((char *)(y) - (char *)&(((sf_block *)0x0)->body.payload))),
    1, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    void * z = sf_realloc(x, 0);
    _assert_null_payload_pointer(z);
    _assert_block_info(((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload))),
    0, max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32));

    // after realloc x to (2) z, x is now a free block
    size_t exp_free_sz_x2z = max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32);
    _assert_free_block_count(exp_free_sz_x2z, 1);

    size_t align_start = ((((size_t)(sf_mem_start() + sizeof(sf_header)) + (1<<4) - 1) >>4) <<4);
    char *first_block = (char *)(align_start - sizeof(sf_header)) + 32;
    size_t unused_area_size = ((size_t)first_block - (size_t)sf_mem_start()) + sizeof(sf_header);

    // the size of the remaining free block
    size_t exp_free_sz = PAGE_SZ - exp_free_sz_x2z
    - max(((((size_t)(sz+sizeof(sf_header)) + (1<<4) - 1) >>4) <<4), 32)
    - unused_area_size;
    _assert_free_block_count(exp_free_sz, 1);

    _assert_errno_eq(0);
}

/*
 * Illegal pointer tests.
 */
Test(sf_memsuite_grading, free_null, .signal = SIGABRT, .timeout = TEST_TIMEOUT)
{
    size_t sz = 1;
    (void) sf_malloc(sz);
    sf_free(NULL);
    cr_assert_fail("SIGABRT should have been received");
}

//This test tests: Freeing a memory that was free-ed already
Test(sf_memsuite_grading, free_unallocated, .signal = SIGABRT, .timeout = TEST_TIMEOUT)
{
    size_t sz = 1;
    void *x = sf_malloc(sz);
    sf_free(x);
    sf_free(x);
    cr_assert_fail("SIGABRT should have been received");
}

Test(sf_memsuite_grading, free_block_too_small, .signal = SIGABRT, .timeout = TEST_TIMEOUT)
{
    size_t sz = 1;
    void * x = sf_malloc(sz);

    ((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload)))->header = 0x0UL;

    sf_free(x);
    cr_assert_fail("SIGABRT should have been received");
}

Test(sf_memsuite_grading, free_prev_alloc, .signal = SIGABRT, .timeout = TEST_TIMEOUT)
{
    size_t sz = 1;
    void * w = sf_malloc(sz);
    void * x = sf_malloc(sz);
    ((sf_block *)((char *)(x) - (char *)&(((sf_block *)0x0)->body.payload)))->header &= ~0x2;
    sf_free(x);
    sf_free(w);
    cr_assert_fail("SIGABRT should have been received");
}

// random block assigments. Tried to give equal opportunity for each possible order to appear.
// But if the heap gets populated too quickly, try to make some space by realloc(half) existing
// allocated blocks.
Test(sf_memsuite_grading, stress_test, .timeout = TEST_TIMEOUT)
{
    errno = 0;

    int order_range = 13;
    int nullcount = 0;

    void * tracked[100];

    for (int i = 0; i < 100; i++)
    {
        int order = (rand() % order_range);
        size_t extra = (rand() % (1 << order));
        size_t req_sz = (1 << order) + extra;

        tracked[i] = sf_malloc(req_sz);
        // if there is no free to malloc
        if (tracked[i] == NULL)
        {
            order--;
            while (order >= 0)
            {
                req_sz = (1 << order) + (extra % (1 << order));
                tracked[i] = sf_malloc(req_sz);
                if (tracked[i] != NULL)
                {
                    break;
                }
                else
                {
                    order--;
                }
            }
        }

        // tracked[i] can still be NULL
        if (tracked[i] == NULL)
        {
            nullcount++;
            // It seems like there is not enough space in the heap.
            // Try to halve the size of each existing allocated block in the heap,
            // so that next mallocs possibly get free blocks.
            for (int j = 0; j < i; j++)
            {
                if (tracked[j] == NULL)
                {
                    continue;
                }

                sf_block *bp = ((sf_block *)((char *)(tracked[j]) - (char *)&(((sf_block *)0x0)->body.payload)));
                req_sz = (bp->header & ~((1LU << 4) - 1)) >> 1;
                tracked[j] = sf_realloc(tracked[j], req_sz);
            }
        }
        errno = 0;
    }

    for (int i = 0; i < 100; i++)
    {
        if (tracked[i] != NULL)
        {
            sf_free(tracked[i]);
        }
    }

    _assert_heap_is_valid();

    // As allocations are random, there is a small probability that the entire heap
    // has not been used.  So only assert that there is one free block, not what size it is.
    //size_t exp_free_sz = MAX_SIZE - sizeof(_sf_prologue) - sizeof(_sf_epilogue);
    /*
     * We can't assert that there's only one big free block anymore because of
     * the quick lists. Blocks inside quick lists are still marked "allocated" so
     * they won't be coalesced with adjacent blocks.
     *
     * Let's comment this out. After all, _assert_heap_is_valid() will check uncoalesced
     * adjacent free blocks anyway.
     */
    // _assert_free_block_count(0, 1);
}
