#include <stdio.h>
#include "../include/sfmm.h"

void test_mid_of_list();

void check_no_splinter_creation();

void check_heap_extension() ;

void check_free_errors();

void check_coal();

void test2();

void newtest();

void sbrk_back_coalesce_check();

void realloc_test();

void check_big_block_test();

int main(int argc, char const *argv[]) {
    sf_mem_init();

//    check_coal();
//    print_heap_overview();
//    test_mid_of_list();
//    check_no_splinter_creation();
//    check_no_splinter_creation();
//    check_heap_extension();
//    check_free_errors();
//    sf_snapshot();
//    test2();
//    newtest();
//    sbrk_back_coalesce_check();
//    realloc_test();
    check_big_block_test();

    sf_mem_fini();
    return EXIT_SUCCESS;
}

void check_big_block_test(){
    sf_malloc(2*PAGE_SZ);
    sf_malloc(PAGE_SZ);
}

void realloc_test(){
    double* ptr = sf_malloc(65);
    *ptr = 10;
    sf_snapshot();
    double* nptr = sf_realloc(ptr, 32);
    printf("\nnew val = %d", (int) *nptr);
    sf_snapshot();
}

void sbrk_back_coalesce_check(){
//    int* f = sf_malloc(120);*f=0;
    double* big_block = sf_malloc(PAGE_SZ*3);
    *big_block=9;
    print_free_list();
    sf_free(big_block);
    print_free_list();

    int i;
    for (i = 0; i < FREE_LIST_COUNT; i++) {
        sf_free_header *fh = (sf_free_header *) (seg_free_list[i].head);
        if (fh != NULL)
            printf("%d,%d",seg_free_list[i].head->header.block_size<<4, 3*PAGE_SZ);
    }
}

void newtest(){
    sf_snapshot();
    double* p1 = sf_malloc(3000);
    sf_snapshot();
    double* p2 = sf_malloc(32);
    sf_snapshot();

    sf_free(p2);
    sf_snapshot();
    sf_free(p1);
    sf_snapshot();
}

void check_coal(){
    int* f1 = sf_malloc(10);*f1=0;
    int* f2 = sf_malloc(1000);*f2=0;
    int* f3 = sf_malloc(10);*f3=0;
    int* f4 = sf_malloc(10);*f4=0;
    int* f5 = sf_malloc(10);*f5=0;
    sf_snapshot();
    sf_free(f5);
    sf_snapshot();
    sf_free(f4);
    sf_snapshot();
    sf_free(f3);
    sf_snapshot();
    sf_free(f2);
    sf_snapshot();
    sf_free(f1);
    sf_snapshot();
}

void check_free_errors(){
    int* f = sf_malloc(4);
    *f =0;

    sf_free(f);
    sf_snapshot();
    sf_free(f);
}

void check_heap_extension() {
    int* g0 = sf_malloc(4060);
    *g0 = 9;
    int* g1 = sf_malloc(4060);
    *g1 = 9;
    int* g2 = sf_malloc(4060);
    *g2 = 9;
    int* g3 = sf_malloc(4060);
    *g3 = 9;
//    int* h = sf_malloc(4060);
//    *h = 0;
    printf("h");
}

void check_no_splinter_creation(){
    double *ptr = sf_malloc(10);
    sf_snapshot();
    *ptr = 100;

    double *ptr1 = sf_malloc(10);
    *ptr1 = 100;
    sf_snapshot();

    sf_free(ptr);
    sf_snapshot();

    sf_free(ptr1);
    sf_snapshot();

    double *ptr2 = sf_malloc(4);
    *ptr2 = 100;
    sf_snapshot();

    sf_free(ptr2);
    sf_snapshot();
}

void test_mid_of_list(){

    double* ptr = sf_malloc(10);
    printf("\nallocated ptr : %p", ptr);
    double* ptr1 = sf_malloc(64);
    printf("\nallocated ptr : %p", ptr1);
    double* ptr2 = sf_malloc(32);
    printf("\nallocated ptr : %p", ptr2);
    double* ptr3 = sf_malloc(32);
    printf("\nallocated ptr : %p", ptr3);
    double* ptr4 = sf_malloc(32);
    printf("\nallocated ptr : %p", ptr4);
    *ptr = 199.82;
    *ptr1 = 320.00;
    *ptr2 = 321.00;
    *ptr3 = 322.00;
    *ptr4 = 323.00;


    sf_free(ptr);
    sf_free(ptr1);
    sf_free(ptr2);
    sf_free(ptr3);
    sf_free(ptr4);

    sf_snapshot();
    printf("\n\nMallocing new element now\n\n");
    double* ptr7 = sf_malloc(43);
    *ptr7 = 320.00;
    sf_snapshot();

    sf_free(ptr7);

    sf_snapshot();
    /* *
     * This should Ideally parse through  list 1 to report one of the middle blocks.
     * */

}
