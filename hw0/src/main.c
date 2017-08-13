
#include "hi.h" /* This is our headerfile, containing the definition for say_hi() */
/**
 * This is the full definition of a main function in C the beginning of our
 * program
 * it is the equivalent of public static void main in java
 *
 * In C main always returns an int, it represents the exit status of your
 * program
 * a program that runs successfully returns 0 one that has an error returns any
 * positive value
 *
 * The arguments passed to main are as follows:
 * - int argc: this is the size of the argv array
 * - char const *argv[] is an array of strings that are the command line
 *   arguments for the program
 * - char const *envp[] are the environment variables availible to any program
 *   run in the OS's environment
 * You can define main to have any of these arguments removed `int main()` for
 * example is entirely valid.
 */

void* noop(void* v){return NULL;}

int main(int argc, char const *argv[], char const *envp[]) {
    /*
     * say_hi() is a function that returns a character pointer (a.k.a a string)
     * Journey on over to `src/hi.c` for the next comments.
     */
    char *hello = say_hi();

    /*
     * This is a print statement in C it places the arguments
     * following the format string into a string following the
     * format string's design.
     */
    printf("%s, %s%c\n", hello, "World", '!');

    /* End of our program */
    return 0;
}
/* Head over to tests/test.c to see what a unit test looks like */
