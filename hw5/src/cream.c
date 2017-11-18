#include <queue.h>
#include <stdio.h>
#include "cream.h"
#include "utils.h"
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
    hashmap_t* hm = create_map(10, jenkins_hash2, map_free_function2);
    map_key_t mk;
    mk.key_len=10;
    mk.key_base = "key";

    map_val_t mv;
    mv.val_len=10;
    mv.val_base= "value";
    
    put(hm, mk, mv, false);
    map_val_t mvt = get(hm, mk);
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
