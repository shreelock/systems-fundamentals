/*
 * DO NOT MODIFY THIS FILE
 * IT WILL BE REPLACED DURING GRADING
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct queue_node_t {
    void *item;
    struct queue_node_t *next;
} queue_node_t;

typedef struct queue_t {
    queue_node_t *front, *rear;
    sem_t items;
    pthread_mutex_t lock;
    bool invalid;
} queue_t;

typedef void (*item_destructor_f)(void *);

/*
 * Creates and returns an instance of a queue
 * and initializes all locks
 *
 * @return A pointer to a queue on the heap
 */
queue_t *create_queue(void);

/*
 * Invalidates a queue from memory and calls destroy_function on all
 * items in the queue
 *
 * @param self The pointer to the queue
 * @param destroy_function The function to call on each item to clean it up
 * @return true if the queue was successfully invalidated, false otherwise
 */
bool invalidate_queue(queue_t *self, item_destructor_f destroy_function);

/*
 * Inserts a pointer to an item at the tail of the queue
 *
 * @param self The pointer to the queue
 * @param item The pointer to insert into the queue
 * @return true if the insertion was successful, false otherwise
 */
bool enqueue(queue_t *self, void *item);

/*
 * Removes and returns the item at the head of the queue
 *
 * @param self The pointer to the queue
 *
 * @return The item in the node at the head of the queue,
 *         or NULL if the queue was empty
 */
void *dequeue(queue_t *self);

#endif
