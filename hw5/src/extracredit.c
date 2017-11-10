#include "utils.h"

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    return NULL;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    return false;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {
	return false;
}

bool invalidate_map(hashmap_t *self) {
    return false;
}
