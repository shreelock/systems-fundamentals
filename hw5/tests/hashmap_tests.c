#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "hashmap.h"
#define NUM_THREADS 100
#define MAP_KEY(kbase, klen) (map_key_t) {.key_base = kbase, .key_len = klen}
#define MAP_VAL(vbase, vlen) (map_val_t) {.val_base = vbase, .val_len = vlen}

hashmap_t *global_map;

typedef struct map_insert_t {
    void *key_ptr;
    void *val_ptr;
} map_insert_t;

/* Used in item destruction */
void map_free_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

uint32_t jenkins_hash(map_key_t map_key) {
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

void map_init(void) {
    global_map = create_map(NUM_THREADS, jenkins_hash, map_free_function);
}

void *thread_put(void *arg) {
    map_insert_t *insert = (map_insert_t *) arg;

    bool op = put(global_map, MAP_KEY(insert->key_ptr, sizeof(int)), MAP_VAL(insert->val_ptr, sizeof(int)), true);
    if (op)
        printf("put val : %d\n", *(int*) insert->val_ptr);
    else
        printf(" couldnt put val ");
    return NULL;
}

void *thread_get(void *arg) {
    map_insert_t *insert = (map_insert_t *) arg;
    map_val_t gotval =  get(global_map, MAP_KEY(insert->key_ptr, sizeof(int)));
    if(gotval.val_base != NULL)
        printf("got val : %d\n", *(int*) gotval.val_base);
    else
        printf("not found\n");
    return NULL;
}

void map_fini(void) {
    invalidate_map(global_map);
}

Test(map_suite, 00_creation, .timeout = 2, .init = map_init, .fini = map_fini) {
    cr_assert_not_null(global_map, "Map returned was NULL");
}

Test(map_suite, 02_multithreaded, .timeout = 2, .init = map_init, .fini = map_fini) {
    pthread_t thread_ids[NUM_THREADS];

    // spawn NUM_THREADS threads to put elements
    for(int index = 0; index < NUM_THREADS; index++) {
        int *key_ptr = malloc(sizeof(int));
        int *val_ptr = malloc(sizeof(int));
        *key_ptr = index;
        *val_ptr = index * 2;

        map_insert_t *insert = malloc(sizeof(map_insert_t));
        insert->key_ptr = key_ptr;
        insert->val_ptr = val_ptr;

        if(pthread_create(&thread_ids[index], NULL, thread_put, insert) != 0)
            exit(EXIT_FAILURE);
    }

    // wait for threads to die before checking queue
    for(int index = 0; index < NUM_THREADS; index++) {
        pthread_join(thread_ids[index], NULL);
    }

    int num_items = global_map->size;
    cr_assert_eq(num_items, NUM_THREADS, "Had %d items in map. Expected %d", num_items, NUM_THREADS);
}

Test(map_suite, 02_multithreaded_act, .timeout = 20, .init = map_init, .fini = map_fini) {
    pthread_t put_thread_ids[NUM_THREADS];
    pthread_t get_thread_ids[NUM_THREADS];
    int idx = 33;
    int *key_ptr = malloc(sizeof(int)), *val_ptr = malloc(sizeof(int));
    *key_ptr = idx, *val_ptr = idx;


    int *key_ptr1 = malloc(sizeof(int)), *val_ptr1 = malloc(sizeof(int));
    *key_ptr1 = idx*3, *val_ptr1 = idx*3;

    map_insert_t *insert = malloc(sizeof(map_insert_t));
    insert->key_ptr = key_ptr, insert->val_ptr = val_ptr;

    map_insert_t *insert1 = malloc(sizeof(map_insert_t));
    insert1->key_ptr = key_ptr1, insert1->val_ptr = val_ptr1;

    // spawn NUM_THREADS threads to put elements
    pthread_create(&put_thread_ids[0], NULL, thread_put, insert);
//    pthread_create(&get_thread_ids[0], NULL, thread_get, insert1);

    for(int index = 1; index < NUM_THREADS; index++) {
        pthread_create(&get_thread_ids[index], NULL, thread_get, insert1);
        pthread_create(&put_thread_ids[index], NULL, thread_put, insert1);
    }


    // wait for threads to die before checking queue
    for(int index = 0; index < NUM_THREADS; index++) {
        pthread_join(put_thread_ids[index], NULL);
        pthread_join(get_thread_ids[index], NULL);
    }

    //int num_items = global_map->size;
    //cr_assert_eq(num_items, 1, "Had %d items in map. Expected %d", num_items, NUM_THREADS);

}
