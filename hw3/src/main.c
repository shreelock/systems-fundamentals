#include <stdio.h>
#include "../include/sfmm.h"

void test_mid_of_list();

int main(int argc, char const *argv[]) {
    sf_mem_init();
    mm_init();
    sf_snapshot();
    test_mid_of_list();

    return EXIT_SUCCESS;
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
    print_free_list();
    sf_free(ptr1);
    print_free_list();
    sf_free(ptr2);
    print_free_list();
    sf_free(ptr3);
    print_free_list();
    sf_free(ptr4);
    print_free_list();

    double* ptr7 = sf_malloc(35);
    *ptr7 = 320.00;
    print_free_list();

    sf_free(ptr7);
    print_free_list();

    /* *
     * This should Ideally parse through  list 1 to report one of the middle blocks.
     * */

}
