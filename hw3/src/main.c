#include <stdio.h>
#include "../include/sfmm.h"

void test_mid_of_list();

void check_no_splinter_creation();

void check_heap_extension() ;

void check_free_errors();

int main(int argc, char const *argv[]) {
    sf_mem_init();

//    print_heap_overview();
//    test_mid_of_list();
//    check_no_splinter_creation();
    check_heap_extension();
//    check_free_errors();
//    sf_snapshot();
    return EXIT_SUCCESS;
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
    int* h = sf_malloc(4060);
    *h = 0;
    printf("h");
}

void check_no_splinter_creation(){
    double *ptr = sf_malloc(10);
    print_free_list();
    *ptr = 100;

    double *ptr1 = sf_malloc(10);
    *ptr1 = 100;
    print_free_list();

    sf_free(ptr);
    print_free_list();

    sf_free(ptr1);
    print_free_list();

    double *ptr2 = sf_malloc(4);
    *ptr2 = 100;
    print_free_list();

    sf_free(ptr2);
    print_free_list();
}

void test_mid_of_list(){

    double* ptr = sf_malloc(10);
    double* ptr1 = sf_malloc(64);
    double* ptr2 = sf_malloc(32);
    double* ptr3 = sf_malloc(32);
    double* ptr4 = sf_malloc(32);
    *ptr = 199.82;
    *ptr1 = 320.00;
    *ptr2 = 320.00;
    *ptr3 = 320.00;
    *ptr4 = 320.00;


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
