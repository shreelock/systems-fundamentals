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
    if (size_ip > 4*PAGE_SZ || size_ip == 0) {
        sf_errno = EINVAL;
        return NULL;
    } else {
        printf("\nRequest is of a valid size.");
    }

    size_t size_hf = size_ip + sizeof(sf_header) + sizeof(sf_footer);
    bool padding_reqd = (int) size_hf % alignment > 0;

    size_t  fin_size = size_hf;
    if (padding_reqd)
        fin_size =  size_hf + ( alignment - (size_hf % alignment) );

    printf("\nFinal size of reqd. block : %d", (int)fin_size);

    // Now check if we have free blocks in any of the free lists
    int minListIdx = getListIndexFromSize(fin_size);
    for (int i = minListIdx ;i<FREE_LIST_COUNT ;i++) {
        sf_free_header* list_head_ptr = seg_free_list[i].head;
        if (list_head_ptr!=NULL){
            printf("\nFound the correct non empty list : %d", i);
            printf("\nlist header is a : %p\n\n", list_head_ptr);
//            sf_blockprint(list_head_ptr);

            /* *
             * Update the list(s) with the updated output_ptr
             * */
            sf_free_header* new_list_head_ptr = list_head_ptr + fin_size;
            new_list_head_ptr->header.block_size = list_head_ptr -> header.block_size - fin_size;
            seg_free_list[i].head = new_list_head_ptr;

            printf("\nnew list head is at : %p\n\n", new_list_head_ptr);
//            sf_blockprint(new_list_head_ptr);
//            sf_snapshot();

            /* *
             * Create a returnable sf_header pointer
             * */
            sf_header* output_ptr = (sf_header*) list_head_ptr;
            output_ptr -> allocated = 1;
            output_ptr -> block_size = fin_size;
            printf("\nIs padding? %d", padding_reqd);
            output_ptr-> padded = (uint64_t) padding_reqd;

            printf("\nreturning pointer is at : %p\n\n", output_ptr + sizeof(sf_header));
//            sf_blockprint(output_ptr);
//            sf_snapshot();

            /* *
             * return output pointer
             * */
            return output_ptr + sizeof(sf_header);
        }
    }

















//    void* base = get_heap_start();
//    sf_header* header = (sf_header*) base;

    return NULL;
}


void *sf_realloc(void *ptr, size_t size) {
    return NULL;
}

void sf_free(void *ptr) {
    return;
}

void mm_init() {
//    fflush(stdout);
    printf("\nmm_initing");

    sf_sbrk();

    void* heap_start = get_heap_start();
    void* heap_end = get_heap_end();
    size_t sizeOfFirstBlock =  heap_end - heap_start;

    printf("\nGot %d size block ", (int)sizeOfFirstBlock);

    int listIdx = getListIndexFromSize(sizeOfFirstBlock);

    printf("\nGot list index as : %d", listIdx);

    seg_free_list[listIdx].head = (sf_free_header*) heap_start;
    seg_free_list[listIdx].head->prev = NULL;
    seg_free_list[listIdx].head->next = NULL;
    seg_free_list[listIdx].head->header.block_size = sizeOfFirstBlock;
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