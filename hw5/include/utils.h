/*
 * DO NOT MODIFY THIS FILE
 * IT WILL BE REPLACED DURING GRADING
 */

#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

#ifdef EC
#include "extracredit.h"
#else
#include "hashmap.h"
#endif

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

uint32_t jenkins_one_at_a_time_hash(map_key_t map_key);
int get_index(hashmap_t *self, map_key_t key);

#endif
