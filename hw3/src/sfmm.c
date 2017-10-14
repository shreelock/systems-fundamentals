/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "../include/sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <asm/errno.h>
#include <jmorecfg.h>

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
    //%%("\n--------------------------------New Allocation----------------------------");
    if (size_ip > 4*PAGE_SZ || size_ip == 0) {
        sf_errno = EINVAL;
        return NULL;
    } else {
        //%%("\nRequest is of a valid size.");
    }

    size_t size_hf = size_ip + sizeof(sf_header) + sizeof(sf_footer);
    uint64_t padding_reqd = (int) size_hf % alignment > 0;

    size_t  fin_size = size_hf;
    if (padding_reqd)
        fin_size =  size_hf + ( alignment - (size_hf % alignment) );

    //%%("\nFinal size of reqd. block : %d", (int)fin_size);

    // Now check if we have free blocks in any of the free lists
    for (int currListIdx = 0 ; currListIdx < FREE_LIST_COUNT; currListIdx++) {
        //%%("\nlist size limits : %d to %d", seg_free_list[currListIdx].min, seg_free_list[currListIdx].max);
        if(fin_size <= seg_free_list[currListIdx].max) {
            //%%("\nChecking for list %d", currListIdx);
            sf_free_header *list_head_ptr = seg_free_list[currListIdx].head;
            if (list_head_ptr != NULL) {
                //%%("\nFound the correct non empty list index : %d", currListIdx);
                //%%("\nlist header is at : %p", list_head_ptr);

                sf_free_header *current_block = seg_free_list[currListIdx].head;
                while (current_block != NULL) {
                    size_t actual_blocksize = current_block->header.block_size <<4;
                    if (actual_blocksize >= fin_size) {

                        //%%("\nBefore Splinting sizes : %d, %d, %d", (int) actual_blocksize, (int) fin_size, (int) (actual_blocksize - fin_size));

                        size_t left_size = actual_blocksize - fin_size;
                        int is_splinter_created = left_size < 4 * sizeof(sf_header);

                        if (is_splinter_created) {
                            // Go full on, if splinter is created.
                            fin_size = actual_blocksize;
                            //%%("\n Splinter will be created, hence size = %d", (int)fin_size);
                        }

                        // Doesn't matter to create. If we'll use it we'll use it.
                        sf_free_header *new_free_block = current_block + fin_size;
                        sf_free_header *current_block_next = current_block->next;
                        sf_free_header *current_block_prev = current_block->prev;

                        /* *
                         * Create a returnable sf_header pointer
                         * */
                        sf_header *output_ptr_header = (sf_header *) current_block;
                        output_ptr_header->allocated = 1;
                        output_ptr_header->block_size = fin_size >> 4;
                        output_ptr_header->padded = (uint64_t) padding_reqd;

                        sf_footer *output_ptr_footer = (void *) current_block + fin_size - sizeof(sf_header);
                        output_ptr_footer->allocated = 1;
                        output_ptr_footer->block_size = output_ptr_header->block_size;
                        output_ptr_footer->padded = (uint64_t) padding_reqd;
                        output_ptr_footer->requested_size = size_ip;


                        //%%("\nAllocated pointer is at : %p", output_ptr_header);
                        void *returning_payload = output_ptr_header + sizeof(sf_header);
                        //%%("\nAllocated payload is at : %p", returning_payload);
                        /* *
                         * Payload is created. Nothing to do here now.
                         * */

                        if (!is_splinter_created) {
                            //%%("\nBeginning to Create a new block");
                            /* *
                             * Create a new block after slicing the older, only if no splinters
                             * */

                            new_free_block->header.block_size = left_size >> 4;
                            //%%("\nNew free block is created at : %p", new_free_block);
                            int newListindex = getListIndexFromSize(left_size);
                            //%%("\nNew free block size : %lu", left_size);
                            //%%("\nNew free block's list index : %d", newListindex);

                            new_free_block->prev = current_block_prev;
                            new_free_block->next = current_block_next;

                            if (seg_free_list[newListindex].head != NULL)
                                seg_free_list[newListindex].head->next = new_free_block;
                            seg_free_list[newListindex].head = new_free_block;

                            //%%("\nNew list head is now pointing at : %p", new_free_block);

                            sf_footer* new_footer =  (void*) new_free_block + left_size - sizeof(sf_header);
                            new_footer->block_size = left_size >> 4;
//                        sf_blockprint(new_free_block);
                        }
                        if (seg_free_list[currListIdx].head == current_block) {
                            seg_free_list[currListIdx].head = current_block->prev;
                            //%%("\nUpdated free list's header");
//                            sf_snapshot();
                        }


                        /* *
                         * return output pointer
                         * */
                        return returning_payload;
                    }
                    current_block = current_block->prev;
                    //%%("\nMoving to the next free block on the list");
                }
            } else {
                //%%("\nGot an empty list : %d", currListIdx);
            }
        } else {
            //%%("\nCould not satisfy size rule, list : %d", currListIdx);
        }
    }
    return NULL;
}


void *sf_realloc(void *ptr, size_t size) {
    return NULL;
}

void sf_free(void* ptr) {
    fflush(stdout);
    //%%("\n--------------------------------New Freeing motion--------------------------");
    //%%("\nPayload to be freed is at : %p", ptr);
    sf_header* free_block = (sf_header*)ptr - sizeof(sf_header);
    //%%("\nHeader of the freed payload is at : %p", free_block);

    size_t size_of_block = ((free_block->block_size)<<4);
    //%%("\nSize of the block to be freed : %d", (int)size_of_block);

    int listIndex = getListIndexFromSize(size_of_block);
    //%%("\nGot appropriate list index = %d", listIndex);

    sf_free_header* new_free_block = (sf_free_header*) free_block;
    new_free_block->prev = seg_free_list[listIndex].head;
    new_free_block->next = NULL;
    new_free_block->header.block_size = size_of_block>>4;
    new_free_block->header.allocated = 0;
    new_free_block->header.padded = 0;

    sf_footer* new_footer = (void*) free_block + size_of_block - sizeof(sf_header);
    new_footer->block_size = new_free_block->header.block_size;
    new_footer->allocated = 0;
    new_footer->padded = 0;
    new_footer->requested_size = 0;

    if (seg_free_list[listIndex].head != NULL)
        seg_free_list[listIndex].head->next = new_free_block;
    seg_free_list[listIndex].head = new_free_block;

    //%%("\nThe updated head of the list after freeing is at : %p\n\n", seg_free_list[listIndex].head);
//    sf_blockprint(seg_free_list[listIndex].head);

}

void mm_init() {
//    fflush(stdout);
    //%%("\nmm_initing");

    sf_sbrk();

    void* heap_start = get_heap_start();
    void* heap_end = get_heap_end();
    size_t sizeOfFirstBlock =  heap_end - heap_start;

//    //%%("\nGot %d size block ", (int)sizeOfFirstBlock);

    int listIdx = getListIndexFromSize(sizeOfFirstBlock);

//    //%%("\nGot list index as : %d", listIdx);

    seg_free_list[listIdx].head = (sf_free_header*) heap_start;
    seg_free_list[listIdx].head->prev = NULL;
    seg_free_list[listIdx].head->next = NULL;
    seg_free_list[listIdx].head->header.block_size = sizeOfFirstBlock >> 4;
    seg_free_list[listIdx].head->header.allocated = 0;

//    sf_blockprint(seg_free_list[listIdx].head);

    sf_footer* footer = heap_start + sizeOfFirstBlock - sizeof(sf_header);
    footer->block_size = sizeOfFirstBlock>>4;
    footer->allocated = 0;

//    sf_blockprint(footer);

    //%%("\nPut initial block at : %p", seg_free_list[listIdx].head);
    //%%("\nInitial block footer is at : %p", footer);
    //%%("\nSize : %d, %d", (int) (footer - (sf_footer*)heap_start), (int) sizeOfFirstBlock);
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
    //%%("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
}

void print_free_list(){
    for(int i=0;i<4;i++){
        printf("\nList no. %d :", i);
        sf_free_header* block = seg_free_list[i].head;
        while(block!=NULL) {
            printf("\nSize: %d, \tPrev: %p, \tHead: %p, \tNext: %p", (int)block->header.block_size<<4, block->prev, block, block->next);
            block = block->prev;
        }
    }
}