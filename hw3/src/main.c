#include <stdio.h>
#include "../include/sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();
    mm_init();
    sf_snapshot();

    double* ptr = sf_malloc(sizeof(double));
    double* ptr1 = sf_malloc(sizeof(double));
    double* ptr2 = sf_malloc(sizeof(double));
    double* ptr3 = sf_malloc(sizeof(double));
    double* ptr4 = sf_malloc(sizeof(double));
    double* ptr5 = sf_malloc(sizeof(double));
    double* ptr6 = sf_malloc(sizeof(double));
    *ptr = 199.82;
    *ptr1 = 320.00;
    *ptr2 = 320.00;
    *ptr3 = 320.00;
    *ptr4 = 320.00;
    *ptr5 = 320.00;
    *ptr6 = 320.00;


    sf_free(ptr);
    sf_snapshot();
    sf_free(ptr1);
    sf_snapshot();
    sf_free(ptr2);
    sf_snapshot();
    sf_free(ptr3);
    sf_snapshot();
    sf_free(ptr4);
    sf_snapshot();
    sf_free(ptr5);
    sf_snapshot();
    sf_free(ptr6);
    sf_snapshot();

    double* ptr7 = sf_malloc(sizeof(double));
    *ptr7 = 320.00;
    sf_snapshot();

//    double* ptr2 = sf_malloc(50*sizeof(double));
//    double* ptr3 = sf_malloc(50*sizeof(double));
//    double* ptr4 = sf_malloc(50*sizeof(double));
//    double* ptr5 = sf_malloc(50*sizeof(double));
//    double* ptr6 = sf_malloc(50*sizeof(double));
//    double* ptr7 = sf_malloc(50*sizeof(double));

//    *ptr1 = 320.00;
//    *ptr2 = 320.00;
//    *ptr3 = 320.00;
//    *ptr4 = 320.00;
//    *ptr5 = 320.00;
//    *ptr6 = 320.00;
//    *ptr7 = 320.00;

//    printf("\n%f", *ptr3);

//    sf_free(ptr);

//    sf_mem_fini();

    return EXIT_SUCCESS;
}
