#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "utf.h"

/**
 * @brief Error checking for malloc
 *
 * @param size the amount of bytes to allocate
 * @return pointer to memory
 */
void* Malloc(size_t size);

void* Calloc(size_t nmemb, size_t size);

int Open(const char* pathname, int flags);

ssize_t read_to_bigendian(int fd, void* buf, size_t count);

ssize_t write_to_bigendian(int fd, void* buf, size_t count);

void reverse_bytes(void* buf, size_t count);


/**
* @brief Error checking for memset
*
* @param s a pointer to an area in memory to modify
* @param c constant byte to fill the area with
* @param n number of bytes to fill
*/
void* memeset(void* s, int c, size_t n);

/**
* @brief Error checking for memcpy
*
* @param s a pointer to an area in memory to modify
* @param c constant byte to fill the area with
* @param n number of bytes to fill
*/
void* memecpy(void* dest, const void* src, size_t n);