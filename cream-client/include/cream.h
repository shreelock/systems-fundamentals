/*
 * DO NOT MODIFY THIS FILE
 * IT WILL BE REPLACED DURING GRADING
 */

#ifndef CREAM_H
#define CREAM_H
#include <stdint.h>

typedef struct request_header_t {
    uint8_t request_code;
    uint32_t key_size;
    uint32_t value_size;
} __attribute__((packed)) request_header_t;

typedef enum request_codes {
    PUT = 0x01,
    GET = 0x02,
    EVICT = 0x04,
    CLEAR = 0x08
} request_codes;

typedef struct response_header_t {
    uint32_t response_code;
    uint32_t value_size;
} __attribute__((packed)) response_header_t;

typedef enum response_codes {
    OK = 200,
    UNSUPPORTED = 220,
    BAD_REQUEST = 400,
    NOT_FOUND = 404
} response_codes;

#endif
