#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define NL "\n"

#define KNRM "\033[0m"
#define KRED "\033[1;31m"
#define KGRN "\033[1;32m"
#define KYEL "\033[1;33m"
#define KBLU "\033[1;34m"
#define KMAG "\033[1;35m"
#define KCYN "\033[1;36m"
#define KWHT "\033[1;37m"
#define KBWN "\033[0;33m"

#ifndef DEBUG
#define _PRINT(level, fmt, ...)
#else
#define _PRINT(level, fmt, ...)                                                                                        \
    do {                                                                                                               \
        fprintf(                                                                                                       \
            stderr, level ": %s:%s:%d " KNRM fmt NL, __FILE__, __extension__ __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)
#endif

#define debug(S, ...) _PRINT(KMAG "DEBUG", S, ##__VA_ARGS__)
#define info(S, ...) _PRINT(KBLU "INFO", S, ##__VA_ARGS__)
#define warn(S, ...) _PRINT(KYEL "WARN", S, ##__VA_ARGS__)
#define success(S, ...) _PRINT(KGRN "SUCCESS", S, ##__VA_ARGS__)
#define error(S, ...) _PRINT(KRED "ERROR", S, ##__VA_ARGS__)

#endif /* DEBUG_H */
