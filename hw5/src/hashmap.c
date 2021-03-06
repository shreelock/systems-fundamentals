#include <errno.h>
#include <memory.h>
#include "utils.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

int get_node_index(hashmap_t *self, map_key_t ikey) ;

void unlock_write_lock(hashmap_t *self) ;

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
    if( capacity==0
        || hash_function==NULL
        || destroy_function==NULL
        || hmap->nodes == NULL  ) return NULL;
    return hmap;
}

int areKeysSame(map_key_t k1, map_key_t k2){
    if(k1.key_len==k2.key_len) {
        if (memcmp(k1.key_base, k2.key_base, k1.key_len) == 0) {
            return 1;
        }
    }
    return 0;
}

bool put(hashmap_t *self, map_key_t inkey, map_val_t inval, bool force) {
    if(self==NULL ||
       self->invalid == true ||
       inkey.key_base == NULL ||
       inkey.key_len == 0 ||
       inval.val_base == NULL ||
       inval.val_len == 0){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);

    int index = get_index(self, inkey);
    if(self->capacity > self->size)
        force = false;

    map_node_t *foundnode = self->nodes + index;
    map_key_t foundkey = foundnode->key;

    // If we reached capacity
    int searched_index = 0;
    if(self->capacity == self->size){
        if(force == true) {
            if(areKeysSame(foundkey, inkey)) {
                // We do not change anything. The value will be replaced.
            } else if ( (searched_index = get_node_index(self, inkey)) != -1) {
                // We search for the node everywhere else
                index  = searched_index;
            }

            foundnode = self->nodes + index;

            //regular stuff
            if(foundnode->tombstone == false) {
                // there is a non dead node at this position
                // we have to evict it first, then overwrite
                self->destroy_function(foundnode->key, foundnode->val);
            }
            //overwriting the index value
            foundnode->key = inkey;
            foundnode->val = inval;
            foundnode->tombstone = false;

            pthread_mutex_unlock(&self->write_lock);
            return true;
        } else {
            //throwing error
            errno = ENOMEM;

            pthread_mutex_unlock(&self->write_lock);
            return false;
        }
    }

    // This means we found an empty slot.
    if(foundkey.key_base==NULL) {
        foundnode->key = inkey;
        foundnode->val = inval;
        foundnode->tombstone = false;
        self->size++;

        pthread_mutex_unlock(&self->write_lock);
        return true;
    }
    // if keys are same, update.
    else if(areKeysSame(foundkey, inkey)) {
        foundnode->val = inval;
        foundnode->tombstone = false;

        pthread_mutex_unlock(&self->write_lock);
        return true;
    }
        // Else, there is already something in that positions
        // so, check for consecutive locations in the hashmap.
    else {
        int oldindex = index;
        index++;
        // We are making sure that we didnt reach the old location.
        while ((index = index%self->capacity) != oldindex){
            foundnode = self->nodes + index;
            foundkey = foundnode->key;

            // This means we found an empty slot
            // Works for tombstones as well
            if(foundkey.key_base==NULL) {
                foundnode->key = inkey;
                foundnode->val = inval;
                foundnode->tombstone = false; // We made the node ALIVE now
                self->size++;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
                // if keys is not null and same, update.
            else if(areKeysSame(foundkey, inkey)) {
                foundnode->val = inval;
                foundnode->tombstone = false;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
            index++;
        }
        // we traversed the whole array for that key, and it was no where to be
        // found. return the null value.
    }
    pthread_mutex_unlock(&self->write_lock);
    return false;
}

map_val_t get(hashmap_t *self, map_key_t inkey) {
    if(self==NULL ||
       self->invalid == true ||
       inkey.key_base == NULL ||
       inkey.key_len == 0){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if(self->num_readers == 1) //First reader is in.
        pthread_mutex_lock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);


    int index = get_index(self, inkey);
    map_node_t *foundnode = self->nodes + index;
    map_key_t foundkey = foundnode->key;
    map_val_t foundval = foundnode->val;
    // if keys are same, check if val is not null and return accordingly.
    if(areKeysSame(foundkey, inkey)) {
            // We found the node, if no reader is left, leave the write lock
            unlock_write_lock(self);
            return MAP_VAL(foundval.val_base, foundval.val_len);
    }
        // Else, check for consecutive locations in the hashmap.
    else {
        int oldindex = index;
        index++;
        while ((index = index%self->capacity) != oldindex){
            foundnode = self->nodes + index;
            foundkey = foundnode->key;
            foundval = foundnode->val;

            //If keys are same, check if val is not null and return accordingly.
            if(areKeysSame(foundkey, inkey)) {
                    unlock_write_lock(self);
                    return MAP_VAL(foundval.val_base, foundval.val_len);
            }
            else if (foundnode->tombstone == true) {
                // Move along, go to next step fro increasing index.
            }
                // Else if Key itself is null, i.e. no item is present,
                // then we found an empty space in the later spaces and
                // we couldn't find out requested key.
            else if (foundkey.key_base == NULL) {
                unlock_write_lock(self);
                return MAP_VAL(NULL, 0);
            }
            index++;
        }
        // we traversed the whole array for that key, and it was no where to be
        // found. return the null value.
    }
    unlock_write_lock(self);
    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t inkey) {
    if(self==NULL ||
       self->invalid == true ||
       inkey.key_base == NULL ||
       inkey.key_len == 0){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
    pthread_mutex_lock(&self->write_lock);
    int searched_index = get_node_index(self, inkey);
    if (searched_index != -1) {
        map_node_t* node = self->nodes + searched_index;
        map_node_t nodetoreturn = MAP_NODE(node->key, node->val, true);
        node->key = MAP_KEY(NULL, 0);
        node->val = MAP_VAL(NULL, 0);
        node->tombstone = true;
        self->size--;

        pthread_mutex_unlock(&self->write_lock);
        return nodetoreturn;
    }

    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

void unlock_write_lock(hashmap_t *self) {
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);
}

bool clear_map(hashmap_t *self) {
    if(self==NULL || self->invalid == true){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);
    for (int i=0;i<self->capacity;i++){
        map_node_t* node = self->nodes + i;
        if(node->key.key_base !=NULL && node->tombstone == false){
            self->destroy_function(node->key, node->val);
        }
        node->key = MAP_KEY(NULL, 0);
        node->val = MAP_VAL(NULL, 0);
        node->tombstone = false;
    }
    self->size = 0;

    pthread_mutex_unlock(&self->write_lock);
    return true;
}

bool invalidate_map(hashmap_t *self) {
    if(self==NULL || self->invalid == true){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);
    for (int i=0;i<self->capacity;i++){
        map_node_t* node = self->nodes + i;
        if(node->key.key_base !=NULL && node->tombstone == false){
            self->destroy_function(node->key, node->val);
        }
    }
    free(self->nodes);
    self->invalid = true;
    self->size = 0;

    pthread_mutex_unlock(&self->write_lock);
    return true;
}

int get_node_index(hashmap_t *self, map_key_t inkey) {
    //We dont need EINVAL check
    // We dont need mutex checks as well. Since we are doing it under them

    int index = get_index(self, inkey);
    map_node_t *foundnode = self->nodes + index;
    map_key_t foundkey = foundnode->key;
    // if keys are same, check if val is not null and return accordingly.
    if(areKeysSame(foundkey, inkey)) {
        return index;
    }
        // Else, check for consecutive locations in the hashmap.
    else {
        int oldindex = index;
        index++;
        while ((index = index%self->capacity) != oldindex){
            foundnode = self->nodes + index;
            foundkey = foundnode->key;

            //If keys are same, return accordingly.
            if(areKeysSame(foundkey, inkey)) {
                // check if the wanted node is dead, then return Null
                return index;
            }
            else if (foundnode->tombstone == true) {
                // Move along, go to next step by increasing index.
            }
                // Now node is not a tombstone and value is NULL
                // then we found an empty space in the later spaces and
                // we couldn't find out requested key.
            else if (foundkey.key_base == NULL) {
                return -1;
            }
            index++;
        }
        // we traversed the whole array for that key, and it was no where to be
        // found. return the null value.
    }
    return -1;
}