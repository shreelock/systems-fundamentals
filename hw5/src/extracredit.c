#include <errno.h>
#include <memory.h>
#include <extracredit.h>
#include "utils.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

void make_everyone_old_by_one(hashmap_t *self) ;

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

bool put(hashmap_t *self, map_key_t ikey, map_val_t ival, bool force) {
    if(self==NULL ||
       self->invalid == true ||
       ikey.key_base == NULL ||
       ikey.key_len == 0 ||
       ival.val_base == NULL ||
       ival.val_len == 0){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);

    int index = get_index(self, ikey);
    if(self->capacity > self->size)
        force = false;

    map_node_t *foundnode = self->nodes + index;
    map_key_t skey = foundnode->key;

    // If we reached capacity
    if(self->capacity == self->size){
        if(force == true) {
            if(foundnode->tombstone == false) {
                // there is a non dead node at this position
                // we have to evict it first, then overwrite
                self->destroy_function(foundnode->key, foundnode->val);
            }
            //increase the ages of all non null nodes, if this node isn't MRU.
            if(foundnode->age!=-1)
                make_everyone_old_by_one(self);

            //overwriting the index value
            foundnode->key = ikey;
            foundnode->val = ival;
            foundnode->tombstone = false;

            // making this item the most recently used
            foundnode->age = -1;

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
    if(skey.key_base==NULL) {
        // make everyone old by one
        if(foundnode->age != -1)
            make_everyone_old_by_one(self);

        foundnode->key = ikey;
        foundnode->val = ival;
        foundnode->tombstone = false;

        // mark this node MRU
        foundnode->age = -1;
        self->size++;

        pthread_mutex_unlock(&self->write_lock);
        return true;
    }
    // if keys are same, update.
    else if(areKeysSame(skey, ikey)) {
        // make everyone old by one
        if (foundnode->age != -1)
            make_everyone_old_by_one(self);

        foundnode->val = ival;
        foundnode->tombstone = false;

        // mark this node MRU
        foundnode->age = -1;

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
            map_node_t *node = self->nodes + index;
            map_key_t tkey = node->key;

            //If keys are same for some next index, overwrite.
            if(areKeysSame(tkey, ikey)) {
                //make old
                if (node->age != -1)
                    make_everyone_old_by_one(self);

                node->val = ival;
                node->tombstone=false;

                //set MRU
                node->age = -1;

                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
                // Else if Key itself is null, i.e. no item is present,
                // then we found an empty space in the later spaces and
                // we are assigning it to the key
            else if (tkey.key_base == NULL) {
                //make old
                if(node->age != -1)
                    make_everyone_old_by_one(self);

                node->key = ikey;
                node->val = ival;
                node->tombstone=false;

                //make MRU
                node->age = -1;

                self->size++;

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

map_val_t get(hashmap_t *self, map_key_t ikey) {
    if(self==NULL ||
       self->invalid == true ||
       ikey.key_base == NULL ||
       ikey.key_len == 0){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    pthread_mutex_lock(&self->fields_lock);

    self->num_readers++;
    if(self->num_readers == 1) //First reader is in.
        pthread_mutex_lock(&self->write_lock);

    pthread_mutex_unlock(&self->fields_lock);


    int index = get_index(self, ikey);
    map_node_t *foundnode = self->nodes + index;
    map_key_t skey = foundnode->key;
    map_val_t sval = foundnode->val;
    // if keys are same, check if val is not null and return accordingly.
    if(areKeysSame(skey, ikey)) {
        if(sval.val_base != NULL) {
            //make old
            if(foundnode->age != -1)
                make_everyone_old_by_one(self);
            //make MRU
            foundnode->age = -1;

            // We found the node, if no reader is left, leave the write lock
            pthread_mutex_lock(&self->fields_lock);
            self->num_readers--;
            if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
            pthread_mutex_unlock(&self->fields_lock);

            return MAP_VAL(sval.val_base, sval.val_len);
        }
        else {

            pthread_mutex_lock(&self->fields_lock);
            self->num_readers--;
            if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
            pthread_mutex_unlock(&self->fields_lock);
            return MAP_VAL(NULL, 0);
        }
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
            if(areKeysSame(tkey, ikey)) {
                if(tval.val_base!=NULL) {
                    //make old
                    if(foundnode->age != -1)
                        make_everyone_old_by_one(self);
                    //make MRU
                    foundnode->age = -1;

                    // We found the node, if no reader is left, leave the write lock
                    pthread_mutex_lock(&self->fields_lock);
                    self->num_readers--;
                    if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
                    pthread_mutex_unlock(&self->fields_lock);
                    return MAP_VAL(tval.val_base, tval.val_len);
                }
                else {

                    pthread_mutex_lock(&self->fields_lock);
                    self->num_readers--;
                    if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
                    pthread_mutex_unlock(&self->fields_lock);
                    return MAP_VAL(NULL, 0);
                }
            }
                // Else if Key itself is null, i.e. no item is present,
                // then we found an empty space in the later spaces and
                // we couldn't find out requested key.
            else if (tkey.key_base == NULL) {

                pthread_mutex_lock(&self->fields_lock);
                self->num_readers--;
                if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
                pthread_mutex_unlock(&self->fields_lock);
                return MAP_VAL(NULL, 0);
            }
        }
        // we traversed the whole array for that key, and it was no where to be
        // found. return the null value.
    }

    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0) pthread_mutex_unlock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);
    return MAP_VAL(NULL, 0);
}

//TODO change the complexity to O(k) by adding get logic
map_node_t delete(hashmap_t *self, map_key_t ikey) {
    if(self==NULL ||
       self->invalid == true ||
       ikey.key_base == NULL ||
       ikey.key_len == 0){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
    pthread_mutex_lock(&self->write_lock);
    for(int i=0;i<self->capacity;i++){
        map_node_t* node = self->nodes + i;
        if(areKeysSame(node->key, ikey)){
            map_node_t nodetoreturn = MAP_NODE(node->key, node->val, true);
            node->key = MAP_KEY(NULL, 0);
            node->val = MAP_VAL(NULL, 0);
            node->tombstone = true;
            node->age = 0;
            self->size--;

            pthread_mutex_unlock(&self->write_lock);
            return nodetoreturn;
        }
    }

    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
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
        node->age = 0;
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

int find_idx_of_LRU_elem(hashmap_t* self) {
    // will always be called with mutex locked, so no need of mutex management
    int max_age_idx = 0;
    long max_age = -2;
    for (int i=0; i<self->capacity; i++) {
        map_node_t* node = self->nodes + i;
        if(node->key.key_base !=NULL && node->tombstone == false){
            if (node->age > max_age){
                max_age_idx = i;
                max_age = node->age;
            }
        }
    }
    return max_age_idx;
}

void make_everyone_old_by_one(hashmap_t *self) {
    if(self==NULL || self->invalid == true){
        errno = EINVAL;
        return;
    }
    // will always be called with mutex locked, so no need of mutex management
    for (int i=0; i<self->capacity; i++) {
        map_node_t* node = self->nodes + i;
        if(node->key.key_base !=NULL && node->tombstone == false){
            node->age = node->age + 1;
        }
    }
}