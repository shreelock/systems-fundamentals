/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef SFMM_H
#define SFMM_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define HEADER_UNUSED_BITS 32
#define BLOCK_SIZE_BITS 28
#define TWO_ZEROES 2
#define PADDED_BITS 1
#define ALLOCATED_BITS 1
#define REQUESTED_SIZE_BITS 32

#define SF_HEADER_SIZE (HEADER_UNUSED_BITS + BLOCK_SIZE_BITS + TWO_ZEROES    \
                        + PADDED_BITS + ALLOCATED_BITS)

#define SF_FOOTER_SIZE SF_HEADER_SIZE

/* Max block sizes for each list, inclusive */
/* THESE SIZE CONSTANTS MAY CHANGE */
#define LIST_1_MIN 32
#define LIST_1_MAX 128
#define LIST_2_MIN 129
#define LIST_2_MAX 512
#define LIST_3_MIN 513
#define LIST_3_MAX 2048
#define LIST_4_MIN 2049
#define LIST_4_MAX -1
#define PAGE_SZ 4096

#define FREE_LIST_COUNT 4

/*

                                      Format of a memory block
    +-----------------------------------------------------------------------------------------+
    |                                       64-bits wide                                      |
    +-----------------------------------------------------------------------------------------+

    +--------------------------------------------+------------------+-------+-------+---------+ <- header
    |                 unused                     |   block_size     |       | padded|  alloc  |
    |                                            |                  |  00   |   1   |    1    |
    |                 32 bits                    |     28 bits      |       |  bit  |   bit   |
    +--------------------------------------------+------------------+-------+-------+---------+
    |                                                                                         | Content of
    |                                   Payload and Padding                                   | the payload
    |                                     (N Memory Rows)                                     |
    |                                                                                         |
    |                                                                                         |
    +--------------------------------------------+------------------+-------+-------+---------+ <- footer
    |              requested_size                |   block_size     |       | padded|  alloc  |
    |                                            |                  |  00   |   1   |    1    |
    |                 32 bits                    |     28 bits      |       |  bit  |   bit   |
    +--------------------------------------------+------------------+-------+-------+---------+
*/

/* Struct for an allocated block header */
typedef struct {
    uint64_t      allocated : ALLOCATED_BITS;
    uint64_t         padded : PADDED_BITS;
    uint64_t     two_zeroes : TWO_ZEROES;
    uint64_t     block_size : BLOCK_SIZE_BITS;
    uint64_t         unused : HEADER_UNUSED_BITS;
} __attribute__((packed)) sf_header;

/* Struct for a block footer */
typedef struct {
    uint64_t      allocated : ALLOCATED_BITS;
    uint64_t         padded : PADDED_BITS;
    uint64_t     two_zeroes : TWO_ZEROES;
    uint64_t     block_size : BLOCK_SIZE_BITS;
    uint64_t requested_size : REQUESTED_SIZE_BITS;
} __attribute__((packed)) sf_footer;

/* Struct for a free block header */
struct sf_free_header{
    sf_header header;
    struct sf_free_header *next;
    struct sf_free_header *prev;
};

typedef struct sf_free_header sf_free_header;

/* Segregated free list struct */
typedef struct {
    sf_free_header *head;
    uint16_t min;
    uint16_t max;
} free_list;

/* sf_errno: will be set on error */
extern int sf_errno;

/* This is the segregated free list */
extern free_list seg_free_list[FREE_LIST_COUNT];

/*
 * @return The starting address of the heap for your allocator.
 */
void *get_heap_start();

/*
 * @return The ending address of the heap for your allocator.
 */
void *get_heap_end();

/*
 * This is your implementation of sf_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, the pointer to a valid region of memory of the
 * requested size is returned, else NULL is returned and sf_errno as follows:
 *
 * If size is invalid (0 or greater than 4 pages), sf_errno is set to EINVAL
 * If the request cannot be satisfied, sf_errno is set to ENOMEM
 */
void *sf_malloc(size_t size);

/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, the pointer to a valid region of memory is
 * returned, else NULL is returned and sf_errno is set appropriately.
 *
 * If there is no memory available sf_realloc should set sf_errno to ENOMEM.
 * If sf_realloc is called with an invalid pointer sf_errno should be set to EINVAL.
 *
 * If sf_realloc is called with a valid pointer and a size of 0 it should free
 * the allocated block and return NULL.
 */
void *sf_realloc(void *ptr, size_t size);

/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function sf_malloc.
 *
 * If ptr is invalid, the function calls abort() to exit the program.
 */
void sf_free(void *ptr);

/* sfutil.c: Helper functions already created for this assignment. */

/*
 * Any program using the sfmm library must call this function ONCE
 * before issuing any allocation requests. This function DOES NOT
 * allocate any space to your allocator.
 */
void sf_mem_init();

/*
 * Any program using the sfmm library must call this function ONCE
 * after all allocation requests are complete. If implemented cleanly,
 * your program should have no memory leaks in valgrind after this function
 * is called.
 */
void sf_mem_fini();

/*
 * This function changes the position of your program's break.
 * Calling this function increments the break by 1 page and updates
 * the heap start and end variables, which can be accessed through
 * get_heap_start() and get_heap_end().
 *
 * @return On success, this function returns the previous program break.
 * If the break was increased, this value is a pointer to the start of the newly
 * allocated memory. On error, (void *) -1 is returned and sf_errno is set to ENOMEM.
 */
void *sf_sbrk();

/*
 * Function which outputs the state of the free-lists to stderr.
 * Performs checks on the placement of the header and footer
 * See sf_snapshot section for details on output format.
 */
void sf_snapshot();

/*
 * Function which prints human readable block format
 *
 * Note: If there are 'bad' addresses in your free list
 * this function will most likely segfault.
 *
 * @param block Address of the block header in memory.
 */
void sf_blockprint(void* block);

/*
 * Prints human readable block format from the address of the payload.
 * IE. subtracts header size from the data pointer to obtain the address
 * of the block header. Calls sf_blockprint internally to print.
 *
 * Note: If there are 'bad' addresses in your free list
 * this function will most likely segfault.
 *
 * @param data Pointer to payload data in memory
 * (value returned by sf_malloc).
 */
void sf_varprint(void *data);

#endif
