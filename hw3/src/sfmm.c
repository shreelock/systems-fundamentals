/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "../include/sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <asm/errno.h>

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};
int alignment = 16;
int sf_errno = 0;

int getListIndexFromSize(size_t sz) ;

/*
 * Initially, if all the free lists are empty, then we allocate from the mem sbrk,
 * set its header as allocated and return that. when we free it later on, then we
 * put it on one of the free lists. that is when free lists get updated.
 *
 * */

void *sf_malloc(size_t size_ip) {
    /*
     * 1. first check if request size is invalid. If it is 0, or grater than 4*PAGESIZE, return Null and set errno as EINVAL
     * 2. then calculate the approximate block size by adding header, footer, and necessary padding to find necessary block size
     * 3. search in all the lists for appropriate block availability. if still not available, return null and    ENOMEM
     */
    fflush(stdout);
    printf("\n--------------------------------New Allocation----------------------------");
    if (size_ip > 4*PAGE_SZ || size_ip == 0) {
        sf_errno = EINVAL;
        return NULL;
    } else {
        printf("\nRequest is of a valid size.");
    }

    size_t size_hf = size_ip + sizeof(sf_header) + sizeof(sf_footer);
    uint64_t padding_reqd = (int) size_hf % alignment > 0;

    size_t  fin_size = size_hf;
    if (padding_reqd)
        fin_size =  size_hf + ( alignment - (size_hf % alignment) );

    printf("\nFinal size of reqd. block : %d", (int)fin_size);

    // Now check if we have free blocks in any of the free lists
    int minListIdx = getListIndexFromSize(fin_size);
    for (int currListIdx = minListIdx ;
                currListIdx < FREE_LIST_COUNT;
                    currListIdx++) {

        sf_free_header* list_head_ptr = seg_free_list[currListIdx].head;
        if (list_head_ptr!=NULL){
            printf("\nFound the correct non empty list index : %d", currListIdx);
            printf("\nlist header is at : %p", list_head_ptr);

            /* *
             * Create a new block after slicing the older
             * */
//            if (list_head_ptr->header.block_size - fin_size > )
            sf_free_header* new_free_block = list_head_ptr + fin_size;
            size_t new_free_block_size = list_head_ptr -> header.block_size - fin_size;
            new_free_block->header.block_size = new_free_block_size;

            printf("\nNew free block is created at : %p", new_free_block);


            int newListindex = getListIndexFromSize(new_free_block_size);
            printf("\nNew free block size : %lu", new_free_block_size);
            printf("\nNew free block's list index : %d", newListindex);


            if (seg_free_list[currListIdx].head->prev != NULL)
                seg_free_list[currListIdx].head->prev->next = NULL;


            seg_free_list[currListIdx].head = seg_free_list[currListIdx].head->prev;


            new_free_block->prev = seg_free_list[newListindex].head;
            new_free_block->next = NULL;

            if (seg_free_list[newListindex].head != NULL)
                seg_free_list[newListindex].head->next = new_free_block;
            seg_free_list[newListindex].head = new_free_block;

            printf("\nNew list head is now pointing at : %p", new_free_block);
            printf("\nThe updated head of the list is : \n\n");
            sf_blockprint(new_free_block);


            /* *
             * Create a returnable sf_header pointer
             * */
            sf_header* output_ptr_header = (sf_header*) list_head_ptr;
            output_ptr_header -> allocated = 1;
            output_ptr_header -> block_size = fin_size;
            output_ptr_header-> padded = (uint64_t) padding_reqd;

            printf("\nAllocated pointer is at : %p", output_ptr_header);
            void* returning_payload = output_ptr_header + sizeof(sf_header) ;
            printf("\nAllocated payload is at : %p", returning_payload);

            /* *
             * return output pointer
             * */
            return output_ptr_header + sizeof(sf_header);
        }
    }
    return NULL;
}


void *sf_realloc(void *ptr, size_t size) {
    return NULL;
}

void sf_free(void* ptr) {
    fflush(stdout);
    printf("\n--------------------------------New Freeing motion--------------------------");
    printf("\nPayload to be freed is at : %p", ptr);
    sf_header* free_block = (sf_header*)ptr - sizeof(sf_header);
    printf("\nHeader of the freed payload is at : %p", free_block);

    size_t size_of_block = free_block->block_size;
    printf("\nSize of the block to be freed : %d", (int)size_of_block);

    int listIndex = getListIndexFromSize(size_of_block);
    printf("\nGot appropriate list index = %d", listIndex);

    sf_free_header* new_free_block = (sf_free_header*) free_block;
    new_free_block->prev = seg_free_list[listIndex].head;
    new_free_block->next = NULL;
    new_free_block->header.block_size = size_of_block;
    new_free_block->header.allocated = 0;
    new_free_block->header.padded = 0;

    if (seg_free_list[listIndex].head != NULL)
        seg_free_list[listIndex].head->next = new_free_block;
    seg_free_list[listIndex].head = new_free_block;

    printf("\nThe updated head of the list after freeing is at : %p\n\n", seg_free_list[listIndex].head);
    sf_blockprint(seg_free_list[listIndex].head);

}

void mm_init() {
//    fflush(stdout);
    printf("\nmm_initing");

    sf_sbrk();

    void* heap_start = get_heap_start();
    void* heap_end = get_heap_end();
    size_t sizeOfFirstBlock =  heap_end - heap_start;

//    printf("\nGot %d size block ", (int)sizeOfFirstBlock);

    int listIdx = getListIndexFromSize(sizeOfFirstBlock);

//    printf("\nGot list index as : %d", listIdx);

    seg_free_list[listIdx].head = (sf_free_header*) heap_start;
    seg_free_list[listIdx].head->prev = NULL;
    seg_free_list[listIdx].head->next = NULL;
    seg_free_list[listIdx].head->header.block_size = sizeOfFirstBlock;
    printf("\nPut initial block at : %p", seg_free_list[listIdx].head);
}

int getListIndexFromSize(size_t sz) {
    int listno = 0;
    if (sz >= LIST_1_MIN && sz <=LIST_1_MAX) listno = 0;
    else if (sz >= LIST_2_MIN && sz <=LIST_2_MAX) listno = 1;
    else if (sz >= LIST_3_MIN && sz <=LIST_3_MAX) listno = 2;
    else if (sz >= LIST_4_MIN) listno = 3;

    return listno;
}

void print_heap_overview() {
    printf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
}