#include <stdio.h>
#include "../include/sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();
    mm_init();
    sf_snapshot();

    double* ptr = sf_malloc(sizeof(double));
    sf_snapshot();

    *ptr = 320.00;

//    printf("\n%p", (void*)ptr);
//    printf("\n%f", *ptr);

//    sf_free(ptr);

//    sf_mem_fini();

    return EXIT_SUCCESS;
}
