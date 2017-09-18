#ifndef HI_H /* This is a preprocessor macro, it asks the preprocessor if HI_H \
                is NOT defined */
#define HI_H /* Since it is not defined we define it */

/* We do this to prevent cyclical inclusion */

#include <stdio.h> /* This is a C library, this is equivalent to import in java */

char* say_hi(); /* This is a function prototype it defines the return type,
                   name, and arguments */

#endif /* HI_H - This is just the end of the if statement */

/* Journey on over to `src/main.c` for the next comments. */
