#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "queue.h"
#define NUM_THREADS 100

queue_t *global_queue;

/* Used in item destruction */
void queue_free_function(void *item) {
    free(item);
}

void queue_init(void) {
    global_queue = create_queue();
}

void *thread_enqueue(void *arg) {
    enqueue(global_queue, arg);
    return NULL;
}

void queue_fini(void) {
    invalidate_queue(global_queue, queue_free_function);
}

Test(queue_suite, 00_creation, .timeout = 2, .init = queue_init, .fini = queue_fini){
    cr_assert_not_null(global_queue, "Queue returned was null");
}

Test(queue_suite, 01_multithreaded, .timeout = 2, .init = queue_init, .fini = queue_fini) {
    pthread_t thread_ids[NUM_THREADS];

    // spawn NUM_THREADS threads to enqueue elements
    for(int index = 0; index < NUM_THREADS; index++) {
        int *ptr = malloc(sizeof(int));
        *ptr = index;

        if(pthread_create(&thread_ids[index], NULL, thread_enqueue, ptr) != 0)
            exit(EXIT_FAILURE);
    }

    // wait for threads to die before checking queue
    for(int index = 0; index < NUM_THREADS; index++) {
        pthread_join(thread_ids[index], NULL);
    }

    // get number of items in queue
    int num_items;
    if(sem_getvalue(&global_queue->items, &num_items) != 0)
        exit(EXIT_FAILURE);

    cr_assert_eq(num_items, NUM_THREADS, "Had %d items. Expected: %d", num_items, NUM_THREADS);
}

