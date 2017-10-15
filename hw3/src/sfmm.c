/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "../include/sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

int extend_heap() ;

void check_valid_ptr(void *ptr);

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
//    fflush(stdout);
    //%%pintf("\n--------------------------------New Allocation----------------------------");
    //%%pintf("\nGot request size as : %d", (int) size_ip);
    if (size_ip > 4*PAGE_SZ || size_ip == 0) {
//        //%%pintf("\nSize limits exceeded");
        sf_errno = EINVAL;
        return NULL;
    } else {
        //%%pintf("\nRequest is of a valid size.");
    }

    size_t size_hf = size_ip + sizeof(sf_header) + sizeof(sf_footer);
    uint64_t padding_reqd = (int) size_hf % alignment > 0;

    size_t  fin_size = size_hf;
    if (padding_reqd)
        fin_size =  size_hf + ( alignment - (size_hf % alignment) );

    if(fin_size > 4*PAGE_SZ) {
        sf_errno = ENOMEM;
        return NULL;
    }

    //%%pintf("\nFinal size of reqd. block : %d", (int)fin_size);
    TRY_AGAIN:
    // Now check if we have free blocks in any of the free lists
    for (int currListIdx = 0 ; currListIdx < FREE_LIST_COUNT; currListIdx++) {
//        //%%pintf("\nRunning Loop : %d", currListIdx);
//        //%%pintf("\nlist size limits : %d to %d", seg_free_list[currListIdx].min, seg_free_list[currListIdx].max);
        if(fin_size <= seg_free_list[currListIdx].max) {
//            //%%pintf("\nChecking for list %d", currListIdx);
            sf_free_header *list_head_ptr = seg_free_list[currListIdx].head;
            if (list_head_ptr != NULL) {
//                //%%pintf("\nFound the correct non empty list index : %d", currListIdx);
//                //%%pintf("\nlist header is at : %p", list_head_ptr);

                sf_free_header *current_block = seg_free_list[currListIdx].head;
                while (current_block != NULL) {
                    size_t actual_blocksize = current_block->header.block_size <<4;
                    if (actual_blocksize >= fin_size) {

                        //%%pintf("\nBefore Splinting sizes : %d, %d, %d", (int) actual_blocksize, (int) fin_size, (int) (actual_blocksize - fin_size));

                        size_t left_size = actual_blocksize - fin_size;
                        int can_create_new_split = left_size >= 4 * sizeof(sf_header);

                        if (!can_create_new_split) {
                            // Go full on, if splinter is created.
                            fin_size = actual_blocksize;
//                            //%%pintf("\n Splinter will be created, hence allocating full size = %d", (int)fin_size);
                        }

                        // Doesn't matter to create. If we'll use it we'll use it.
                        sf_free_header *new_free_block = (void*) current_block + fin_size;
                        sf_free_header *current_block_next = current_block->next;
                        sf_free_header *current_block_prev = current_block->prev;

                        /* *
                         * Create a returnable sf_header pointer
                         * */
                        sf_header *output_ptr_header = (sf_header *) current_block;
                        output_ptr_header->allocated = 1;
                        output_ptr_header->block_size = fin_size >> 4;
                        output_ptr_header->padded = (uint64_t) padding_reqd;

                        sf_footer *output_ptr_footer = (void *) current_block - sizeof(sf_header) + fin_size;
                        output_ptr_footer->allocated = 1;
                        output_ptr_footer->block_size = output_ptr_header->block_size;
                        output_ptr_footer->padded = (uint64_t) padding_reqd;
                        output_ptr_footer->requested_size = size_ip;

                        //%%pintf("\nAllocated pointer is at : %p", output_ptr_header);
                        void *returning_payload = (void*) output_ptr_header + sizeof(sf_header);
                        //%%pintf("\nAllocated payload is at : %p", returning_payload);
                        /* *
                         * Payload is created. Nothing to do here now.
                         * */

                        if (can_create_new_split) {
                            //%%pintf("\nBeginning to Create a new block");
                            /* *
                             * Create a new block after slicing the older, only if no splinters
                             * */

                            new_free_block->header.block_size = (left_size >> 4);
                            //%%pintf("\nNew free block is created at : %p", new_free_block);
                            int newListindex = getListIndexFromSize(left_size);
                            //%%pintf("\nNew free block size : %lu", left_size);
                            //%%pintf("\nNew free block's list index : %d", newListindex);

                            new_free_block->prev = current_block_prev;
                            new_free_block->next = current_block_next;

                            if (seg_free_list[newListindex].head != NULL)
                                seg_free_list[newListindex].head->prev = new_free_block;
                            seg_free_list[newListindex].head = new_free_block;

                            //%%pintf("\nNew list head is now pointing at : %p", new_free_block);

                            sf_footer* new_footer =  (void*) new_free_block - sizeof(sf_header) + left_size;
                            new_footer->block_size = new_free_block->header.block_size;
                            new_footer->allocated = 0;
                            new_footer->padded = 0;
                            new_footer->requested_size = 0;
//                        sf_blockprint(new_free_block);
                        } else {

                            if (current_block_next !=NULL) {
                                current_block_next->prev = current_block_prev;
                            }

                            if (current_block_prev!=NULL) {
                                current_block_prev->next = current_block_next;
                            }
                        }
                        if (seg_free_list[currListIdx].head == current_block) {
                            seg_free_list[currListIdx].head = current_block->next;
                            //%%pintf("\nUpdated free list's header");
//                            sf_snapshot();
                        }

                        /* *
                         * return output pointer
                         * */
                        return returning_payload;
                    }
                    //%%pintf("\n%p Not appropriate",current_block);
                    //%%pintf("\nMoving to the next free block on the list");
                    current_block = current_block->next;
                }
            } else {
//                //%%pintf("\nGot an empty list");
            }
        } else {
//            //%%pintf("\nCould not satisfy size rule, list : %d", currListIdx);
        }
//        //%%pintf("\nLoop over for %d", currListIdx);
    }
    //%%pintf("\nAll lists are empty. Extracting new Block");
    int f = extend_heap();
    if (f)
        goto TRY_AGAIN;
    else {
        sf_errno = ENOMEM;
        return NULL;
    }
}


void *sf_realloc(void *ptr, size_t size) {
    if(size == 0){
        sf_free(ptr);
        return NULL;
    }
    check_valid_ptr(ptr);
    sf_header* input_block_header = ptr - sizeof(sf_header);
    size_t orig_block_size = (input_block_header->block_size<<4);

    sf_footer* input_block_footer = (void*) input_block_header - sizeof(sf_header) + orig_block_size;
    size_t orig_request_size = input_block_footer->requested_size;

    size_t size_hf = size + sizeof(sf_header) + sizeof(sf_footer);
    uint64_t padding_reqd = (int) size_hf % alignment > 0;
    size_t  req_block_size = size_hf;
    if (padding_reqd)
        req_block_size =  size_hf + ( alignment - (size_hf % alignment) );

    if(req_block_size == orig_block_size){
        if(size == orig_request_size) {
            //%%pintf("\nSame pointr, returning.");
            return ptr;
        } else {
            //%%pintf("\nSame block size, updating requested_size and returning");
            input_block_footer->requested_size = size;
            input_block_footer->block_size = input_block_header->block_size;
            input_block_footer->padded = padding_reqd;
            input_block_header->padded = padding_reqd;
            return ptr;
        }
    }

    if(req_block_size > orig_block_size){
        //%%pintf("\nNew size is more than original size, \nallocating new position");
        void* newptr = sf_malloc(size);
        memcpy(newptr, ptr, orig_request_size);
        sf_free(ptr);
        return newptr;
    }

    if(req_block_size < orig_block_size){
        size_t splinter_size = orig_block_size - req_block_size;
        if (splinter_size >= 4*sizeof(sf_header)) {
            //%%pintf("\nNew size is less than original size, \nallocating new position \ncreating splinter");
            sf_footer *new_block_footer = (void *) input_block_header - sizeof(sf_header) + req_block_size;

            input_block_header->block_size = (req_block_size>>4);
            input_block_header->padded = padding_reqd;
            input_block_header->allocated = 1;

            //%%pintf("\nIBH:%p", input_block_header);

            new_block_footer->block_size = input_block_header->block_size;
            new_block_footer->padded = input_block_header->padded;
            new_block_footer->allocated = input_block_header->allocated;
            new_block_footer->requested_size = size;

            //%%pintf("\nNBF:%p", new_block_footer);

            sf_header *splinter_header = (void*) new_block_footer + sizeof(sf_footer);
            splinter_header->block_size = (splinter_size>>4);
            splinter_header->padded = 1;
            splinter_header->allocated = 1;

            //%%pintf("\nSH:%p", splinter_header);

            input_block_footer->block_size = splinter_header->block_size;
            input_block_footer->padded = 1;
            input_block_footer->allocated = 1;
            input_block_footer->requested_size = 1;

            //%%pintf("\nIBF:%p", input_block_footer);
            void* splinter_ptr = (void*) splinter_header + sizeof(sf_header);
            //%%pintf("\ns-p:%p", splinter_ptr);

            sf_free(splinter_ptr);

            return ptr;
        } else {
            //%%pintf("\nNew size is less than original size, \nallocating new position \nNO splinter");
            input_block_header->padded = padding_reqd;

            input_block_footer->padded = input_block_header->padded;
            input_block_footer->allocated = input_block_header->allocated;
            input_block_footer->block_size = input_block_header->block_size;
            input_block_footer->requested_size = size;

            return ptr;
        }
    }

    return NULL;
}

void check_valid_ptr(void *ptr){
    if (ptr == NULL) {
        //%%pintf("\nabort due to NULL");
        abort();
    }
    sf_header* head = ptr  - sizeof(sf_header);
    size_t real_block_size = ((head->block_size)<<4);
    sf_footer* footer = (void*) head - sizeof(sf_header) + real_block_size;

    if (((void*)head < get_heap_start())/* || ((void*)head > get_heap_end())*/) {
        //%%pintf("\n%p, %p, %p", get_heap_start(), head, get_heap_end());
        //%%pintf("\nabort due to heap limits");
        abort();
    }

    if (head->allocated == 0 || footer->allocated == 0) {
        //%%pintf("\nPtrs:%p,%p", head, footer);
        //%%pintf("\nBits:%d, %d", (int) head->allocated, (int) footer->allocated);
        //%%pintf("\nabort due to allocated bits");
        abort();
    }

    if ((head->padded != footer->padded)
        || (head->block_size!=footer->block_size)) {
        //%%pintf("\nabort due to padded and blocksize");
        abort();
    }

    int test_padded = footer->requested_size + 16 != real_block_size;

    if ((head->padded != test_padded) || (footer->padded != test_padded)) {
        //%%pintf("\nabort due to padded not correct");
        abort();
    }
}

size_t perform_coalescion(void *new_free_block, size_t size_of_block) {
    sf_free_header* next_block = (void*) new_free_block + size_of_block;
    //%%pintf("\nNext free block is located at : %p", next_block);
    if(next_block->header.allocated == 0) {
        //%%pintf("\nNext Block is free!");
        size_of_block = size_of_block + (next_block->header.block_size<<4);

        for (int i=0;i<FREE_LIST_COUNT;i++) {
            if (seg_free_list[i].head == next_block) {
                //%%pintf("\nNext block seg list vals for I=%d: %p, %p, %p",i, seg_free_list[i].head->prev, seg_free_list[i].head,seg_free_list[i].head->next);
                //%%pintf("\nNext block seg list vals for I=%d: %p, %p, %p",i+1, seg_free_list[i+1].head->prev, seg_free_list[i+1].head,seg_free_list[i+1].head->next);
                seg_free_list[i].head = seg_free_list[i].head->next;
                //%%pintf("\nNext block vals : %p", seg_free_list[i].head);//, seg_free_list[i].head->prev, seg_free_list[i].head->next);
            }
        }

        if(next_block->prev!=NULL)
            next_block->prev->next = next_block->next;

        if (next_block->next!=NULL)
            next_block->next->prev = next_block->prev;
    }
    return size_of_block;
}

void sf_free(void* ptr) {
    check_valid_ptr(ptr);
    fflush(stdout);
    //%%pintf("\n--------------------------------New Freeing motion--------------------------");
    //%%pintf("\nPayload to be freed is at : %p", ptr);
    sf_header* free_block = ptr - sizeof(sf_header);
    //%%pintf("\nHeader of the freed payload is at : %p", free_block);

    sf_free_header* new_free_block = (sf_free_header*) free_block;
    size_t size_of_block = ((free_block->block_size)<<4);

    size_of_block = perform_coalescion(new_free_block, size_of_block);

    //%%pintf("\nSize of the block to be freed : %d", (int)size_of_block);

    int listIndex = getListIndexFromSize(size_of_block);
    //%%pintf("\nGot appropriate list index = %d", listIndex);

    new_free_block->next = seg_free_list[listIndex].head;
    new_free_block->prev = NULL;
    new_free_block->header.block_size = size_of_block>>4;
    new_free_block->header.allocated = 0;
    new_free_block->header.padded = 0;

    sf_footer* new_footer = (void*) free_block - sizeof(sf_header) + size_of_block;
    new_footer->block_size = new_free_block->header.block_size;
    new_footer->allocated = 0;
    new_footer->padded = 0;
    new_footer->requested_size = 0;

    if (seg_free_list[listIndex].head != NULL)
        seg_free_list[listIndex].head->prev = new_free_block;
    seg_free_list[listIndex].head = new_free_block;

    //%%pintf("\nThe updated head of the list after freeing is at : %p\n\n", seg_free_list[listIndex].head);
//    sf_blockprint(seg_free_list[listIndex].head);

}


int extend_heap() {
    //%%pintf("\nInside Extend Heap");
    void* old_heap_end = get_heap_end();
    if (old_heap_end == NULL) {
        sf_sbrk();
        old_heap_end = get_heap_start();
    } else {
        void* sbrk_ret = sf_sbrk();
        fflush(stdout);
        if (sbrk_ret == (void *) -1) {
            //%%pintf("\nSBRK returned -1. Abort!");
            return 0;
        }
    }

    void* new_heap_end = get_heap_end();
    size_t sizeOfNewBlock = new_heap_end - old_heap_end;
    if (sizeOfNewBlock != 0) {
        sf_free_header *new_block_header = (sf_free_header *) old_heap_end;
        sf_footer* prev_block_footer = (void*) old_heap_end - sizeof(sf_footer);

        if((void*)prev_block_footer > get_heap_start() && prev_block_footer->allocated == 0) {
            //%%pintf("\nYay! prev block is empty.");
            size_t prev_block_size = prev_block_footer->block_size<<4;
            //%%pintf("\nGot the blocks size as : %d", (int) prev_block_size);
            sf_free_header* prev_block_header = (void*) prev_block_footer - prev_block_size + sizeof(sf_footer);

            sizeOfNewBlock += prev_block_size;

            for (int i=0;i<FREE_LIST_COUNT;i++) {
//                //%%pintf("\nHeaders : %p, %p", seg_free_list[i].head, prev_block_header);
                if (seg_free_list[i].head == prev_block_header) {
                    //%%pintf("\nFound this free list head : %d", i);
                    seg_free_list[i].head = seg_free_list[i].head->next;
                }
            }

            if(prev_block_header->prev!=NULL)
                prev_block_header->prev->next = prev_block_header->next;

            if(prev_block_header->next!=NULL)
                prev_block_header->next->prev = prev_block_header->prev;

            new_block_header = prev_block_header;

        } else {
            //%%pintf("\nNot empty, or beyond limit, proceeding normally");
        }

        int listIndex = getListIndexFromSize(sizeOfNewBlock);

        new_block_header->next = seg_free_list[listIndex].head;
        new_block_header->prev = NULL;
        new_block_header->header.allocated = 0;
        new_block_header->header.padded = 0;
        new_block_header->header.block_size = sizeOfNewBlock >> 4;

        sf_footer *footer = (void *) new_block_header - sizeof(sf_header) + sizeOfNewBlock ;
        footer->block_size = new_block_header->header.block_size;
        footer->padded = 0;
        footer->allocated = 0;
        footer->requested_size = 0;

        seg_free_list[listIndex].head = new_block_header;
        //%%pintf("\n New block Init at : %p", new_block_header);
        //%%pintf("\n New Block Size : %d", (int) new_block_header->header.block_size<<4);

        //%%pintf("\nFinal Memory location is : %p", get_heap_end());
        return 1;
    }

    return 0;
}

int getListIndexFromSize(size_t sz) {
    int listno = 0;
    if (sz >= LIST_1_MIN && sz <=LIST_1_MAX) listno = 0;
    else if (sz >= LIST_2_MIN && sz <=LIST_2_MAX) listno = 1;
    else if (sz >= LIST_3_MIN && sz <=LIST_3_MAX) listno = 2;
    else if (sz >= LIST_4_MIN && sz <=LIST_4_MAX) listno = 3;

    return listno;
}

void print_heap_overview() {
    //%%pintf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
    sf_sbrk();
    //%%pintf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
    sf_sbrk();
    //%%pintf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
    sf_sbrk();
    //%%pintf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
    sf_sbrk();
    //%%pintf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
    sf_sbrk();
    //%%pintf("%p, \t%p, %d\n", get_heap_start(), get_heap_end(), (int) (get_heap_end() - get_heap_start()));
}

void print_free_list(){
    //%%pintf("\n\n\n");
    for(int i=0;i<4;i++){
        //%%pintf("\nList no. %d :", i);
        sf_free_header* block = seg_free_list[i].head;
        while(block!=NULL) {
            //%%pintf("\nSize: %d, \tPrev: %p, \tHead: %p, \tNext: %p", (int)block->header.block_size<<4, block->prev, block, block->next);
            //%%pintf("\nFooter is at : %p", (void*)block + (block->header.block_size<<4) - sizeof(sf_header));
            block = block->next;
        }
    }
}