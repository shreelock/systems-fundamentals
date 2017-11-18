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
    map_key_t mk1 = MAP_KEY(malloc(sizeof(char)), 10);
    map_key_t mk2 = MAP_KEY(malloc(sizeof(char)), 10);
    map_key_t mk3 = MAP_KEY(malloc(sizeof(char)), 10);
    map_key_t mk4 = MAP_KEY(malloc(sizeof(char)), 10);
    map_key_t mk5 = MAP_KEY(malloc(sizeof(char)), 10);

    map_val_t mv1 = MAP_VAL(malloc(sizeof(char)), 10);
    map_val_t mv2 = MAP_VAL(malloc(sizeof(char)), 10);
    map_val_t mv3 = MAP_VAL(malloc(sizeof(char)), 10);
    map_val_t mv4 = MAP_VAL(malloc(sizeof(char)), 10);
    map_val_t mv5 = MAP_VAL(malloc(sizeof(char)), 10);

    put(hm, mk1, mv1, false);
    printf("Putting : %s\n", (char *) mk1.key_base);
    printhashmap(hm);

    put(hm, mk2, mv2, false);
    printf("Putting : %s\n", (char *) mk2.key_base);
    printhashmap(hm);

    put(hm, mk3, mv3, false);
    printf("Putting : %s\n", (char *) mk3.key_base);
    printhashmap(hm);

    put(hm, mk4, mv4, false);
    printf("Putting : %s\n", (char *) mk4.key_base);
    printhashmap(hm);

    printf("Deleting : %s\n", (char *) mk3.key_base);
    delete(hm, mk3);
    printhashmap(hm);

    printf("Deleting : %s\n", (char *) mk3.key_base);
    delete(hm, mk3);
    printhashmap(hm);


    map_val_t mvt = get(hm, mk2);
    printf("%s", (char*) mvt.val_base);

    put(hm, mk5, mv5, true);
    printf("Putting : %s\n", (char *) mk5.key_base);
    printhashmap(hm);

    clear_map(hm);
    printhashmap(hm);
    printf("Cleared the map\n");

    invalidate_map(hm);
    printhashmap(hm);

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
