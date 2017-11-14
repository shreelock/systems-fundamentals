/*
 * DO NOT MODIFY THIS FILE
 * IT WILL BE REPLACED DURING GRADING
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct map_key_t {
    void *key_base;
    size_t key_len;
} map_key_t;

typedef struct map_val_t {
    void *val_base;
    size_t val_len;
} map_val_t;

typedef uint32_t (*hash_func_f)(map_key_t);
typedef void (*destructor_f)(map_key_t, map_val_t);

typedef struct map_node_t {
    map_key_t key;
    map_val_t val;
    bool tombstone;
} map_node_t;

typedef struct hashmap_t {
    uint32_t capacity;
    uint32_t size;
    map_node_t *nodes;
    hash_func_f hash_function;
    destructor_f destroy_function;
    int num_readers;
    pthread_mutex_t write_lock;
    pthread_mutex_t fields_lock;
    bool invalid;
} hashmap_t;

/*
 * Create a new hash map.
 *
 * @param capacity The number of elements the map can hold.
 * @param hash_function The function to be used to hash keys.
 * @param destroy_function The function to be used to destroy elements
 *                         when the map is destroyed.
 * @return A pointer to the new hashmap_t instance.
 */
hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function);

/*
 * Insert a new key/value pair into the map.
 * If the key already exists, the corresponding value is overwritten.
 * If the map is full and force is false, nothing is inserted.
 * If the map is full and force is true, the entry at the index computed by
 * get_index() is overwritten.
 *
 * @param self The hash map to use
 * @param key The key to insert
 * @param val The value to insert
 * @param force Whether or not entries should be overwritten if the map is full.
 * @return true if the insertion was sucessful, false otherwise.
 */
bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force);

/*
 * Retrieve the value associated with a key.
 *
 * @param self The hash map to use
 * @param key The key to search for
 * @return The corresponding value, or a map_val_t instance with a null
 *         pointer and a value length of 0 if the key is not found.
 */
map_val_t get(hashmap_t *self, map_key_t key);

/*
 * Remove the entry associated with a key.
 *
 * @param self The hash map to use
 * @param key The key to remove.
 * @return The removed map_node_t instance.
 */
map_node_t delete(hashmap_t *self, map_key_t key);

/*
 * Clears and destroys all entries in the map.
 *
 * @param self The hash map to clear.
 * @return true if the operation was successful, false otherwise
 */
bool clear_map(hashmap_t *self);

/*
 * Invalidate a hash map and its elements using the destructor function in the
 * map.
 *
 * @param self The hash map to invalidate.
 * @return true if the operation was successful.
 */
bool invalidate_map(hashmap_t *self);

#endif
