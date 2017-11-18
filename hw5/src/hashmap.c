#include <errno.h>
#include "utils.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    struct hashmap_t *hmap = (hashmap_t*) calloc(1, sizeof(hashmap_t));
    hmap->nodes = (map_node_t*)calloc(capacity, sizeof(map_node_t));
    hmap->capacity = capacity;
    hmap->size = 0;
    hmap->hash_function = hash_function;
    hmap->destroy_function = destroy_function;
    hmap->num_readers = 0;
    hmap->invalid = false;
    if(pthread_mutex_init(&hmap->write_lock, NULL)!=0) return NULL;
    if(pthread_mutex_init(&hmap->fields_lock, NULL)!=0) return NULL;
    return hmap;
}

bool put(hashmap_t *self, map_key_t ikey, map_val_t ival, bool force) {
    int index = get_index(self, ikey);
    if(self->capacity > self->size)
        force = false;

    if(self->size == self->capacity && force == false) {
        errno = ENOMEM;
        return false;
    }


    map_node_t *foundnode = self->nodes + index;
    map_key_t skey = foundnode->key;

    // If we reached capacity, overwriting the index value
    if(self->capacity == self->size && force == true){
        foundnode->key = ikey;
        foundnode->val = ival;
        foundnode->tombstone = false;
        return true;
    }

    if(skey.key_base==NULL) {
        foundnode->key = ikey;
        foundnode->val = ival;
        foundnode->tombstone = false;
        self->size++;
        return true;
    }
    // if keys are same, update.
    else if(skey.key_base == ikey.key_base && skey.key_len == ikey.key_len) {
        foundnode->val = ival;
        foundnode->tombstone = false;
        return true;
    }
        // Else, check for consecutive locations in the hashmap.
    else {
        int oldindex = index;
        while (index++ % self->capacity != oldindex){
            map_node_t *node = self->nodes + index;
            map_key_t tkey = node->key;

            //If keys are same for some next index, overwrite.
            if(tkey.key_base == ikey.key_base && tkey.key_len == ikey.key_len) {
                node->val = ival;
                node->tombstone=false;
                return true;
            }
                // Else if Key itself is null, i.e. no item is present,
                // then we found an empty space in the later spaces and
                // we are assigning it to the key
            else if (tkey.key_base == NULL) {
                node->val = ival;
                node->tombstone=false;
                self->size++;
                return true;
            }
        }
        // we traversed the whole array for that key, and it was no where to be
        // found. return the null value.
    }
    return false;
}

map_val_t get(hashmap_t *self, map_key_t ikey) {
    int index = get_index(self, ikey);
    map_node_t *foundnode = self->nodes + index;
    map_key_t skey = foundnode->key;
    map_val_t sval = foundnode->val;
    // if keys are same, check if val is not null and return accordingly.
    if(skey.key_base == ikey.key_base && skey.key_len == ikey.key_len) {
        if(sval.val_base != NULL)
            return MAP_VAL(sval.val_base, sval.val_len);
        else
            return MAP_VAL(NULL, 0);
    }
        // Else, check for consecutive locations in the hashmap.
    else {
        int oldindex = index;
        while (index++ % self->capacity != oldindex){
            map_node_t *node = self->nodes + index;
            // do nothing if we found a tombstone on a node.
            if(node->tombstone)
                continue;

            map_key_t tkey = node->key;
            map_val_t tval = node->val;

            //If keys are same, check if val is not null and return accordingly.
            if(tkey.key_base == ikey.key_base && tkey.key_len == ikey.key_len) {
                if(tval.val_base!=NULL)
                    return MAP_VAL(tval.val_base, tval.val_len);
                else
                    return MAP_VAL(NULL, 0);
            }
                // Else if Key itself is null, i.e. no item is present,
                // then we found an empty space in the later spaces and
                // we couldn't find out requested key.
            else if (tkey.key_base == NULL) {
                return MAP_VAL(NULL, 0);
            }
        }
        // we traversed the whole array for that key, and it was no where to be
        // found. return the null value.
    }
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
