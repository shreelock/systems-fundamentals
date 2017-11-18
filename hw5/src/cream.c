#include <queue.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include "cream.h"
#include "utils.h"

void printhashmap(hashmap_t* hmap);

void queue_free_function1(void *item) {
    free(item);
}

uint32_t jenkins_hash2(map_key_t map_key) {
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

void map_free_function2(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

int main(int argc, char *argv[]) {
    hashmap_t* hm = create_map(3, jenkins_hash2, map_free_function2);
    map_key_t mk1 = MAP_KEY("key1", 10);
    map_key_t mk2 = MAP_KEY("key2", 10);
    map_key_t mk3 = MAP_KEY("key3", 10);
    map_key_t mk4 = MAP_KEY("key4", 10);
    map_key_t mk5 = MAP_KEY("sey5", 10);

    map_val_t mv1 = MAP_VAL("value1", 10);
    map_val_t mv2 = MAP_VAL("value2", 10);
    map_val_t mv3 = MAP_VAL("value3", 10);
    map_val_t mv4 = MAP_VAL("value4", 10);
    map_val_t mv5 = MAP_VAL("value5", 10);

//    put(hm, mk1, mv1, false);
//    printhashmap(hm);
    put(hm, mk2, mv2, false);
    printhashmap(hm);
    put(hm, mk3, mv3, false);
    printhashmap(hm);
    map_node_t n = delete(hm, mk3);
    printf("Deleted : %s\n", n.key);
    map_node_t n2 = delete(hm, mk3);
    printf("Deleted : %s\n", n2.key);
    printhashmap(hm);
    put(hm, mk5, mv5, true);
    printhashmap(hm);


    map_val_t mvt = get(hm, mk2);
    printf("%s", (char*) mvt.val_base);
    exit(0);
}

int queue_test(int argc, char *argv[]) {
    queue_t* gh = create_queue();
    enqueue(gh,"1");
//    enqueue(gh,"2");
//    enqueue(gh,"3");
//    enqueue(gh,"4");
//    printf("%s", (char *) dequeue(gh));
    printf("%s", (char *) dequeue(gh));
    printf("%s", (char *) dequeue(gh));
//    printf("%s", (char *) dequeue(gh));
//    printf("%s", (char *) dequeue(gh));
//    enqueue(gh,"5");
//    invalidate_queue(gh, queue_free_function1);
    exit(0);
}
void printhashmap(hashmap_t* hmap){
    printf("size:%d, capacity:%d\n", hmap->size, hmap->capacity);
    for (int i=0;i<hmap->capacity;i++){
        map_node_t* n = hmap->nodes + i;
        printf("%s:%s\n", (char*) n->key.key_base, (char*) n->val.val_base);
    }
    printf("%s\n", strerror(errno));
    printf("\n");
}
