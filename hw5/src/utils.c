/*
 * DO NOT MODIFY THIS FILE
 * IT WILL BE REPLACED DURING GRADING
 */

#include "utils.h"

/*
 * Computes the hash of a byte stream.
 */
uint32_t jenkins_one_at_a_time_hash(map_key_t map_key) {
    const uint8_t *key = map_key.key_base;
    size_t length = map_key.key_len;
    size_t i = 0;
    uint32_t hash = 0;

    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

/*
 * Gets the index for a key using the hash function
 * in the self parameter.
 */
int get_index(hashmap_t *self, map_key_t key) {
    return self->hash_function(key) % self->capacity;
}
