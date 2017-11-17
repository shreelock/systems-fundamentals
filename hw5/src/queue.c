#include <errno.h>
#include "queue.h"

queue_t *create_queue(void) {
    //When initialised, the queue is empty
    struct queue_t* queue = (queue_t*) calloc(1, sizeof(queue_t));
    if(queue==NULL) return NULL;
    queue->invalid = false; //Just in case
    if(sem_init(&queue->items,0,0)!=0)  return NULL;
    if(pthread_mutex_init(&queue->lock, NULL)!=0)   return NULL;
    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    //https://computing.llnl.gov/tutorials/pthreads/
    if(destroy_function==NULL || self==NULL) {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->lock);

    queue_node_t* elem;
    while((elem = self->front)!=NULL){
        destroy_function(elem->item);
        self->front = elem->next;
        free(elem);
    }
    self->rear = NULL;
    self->invalid = true;

    pthread_mutex_unlock(&self->lock);
    return true;
}

bool enqueue(queue_t *self, void *item) {
    queue_node_t* newnode = calloc(1, sizeof(queue_node_t));
    newnode->item = item;
    newnode->next = NULL;

    pthread_mutex_lock(&self->lock);

    if(self->front==NULL)
        self->front = newnode;

    if(self->rear !=NULL)
        self->rear->next = newnode;

    self->rear = newnode;
    sem_post(&self->items);

    pthread_mutex_unlock(&self->lock);
    return true;
}

void *dequeue(queue_t *self) {

    queue_node_t *node = self->front;

    if(node==NULL) {
        errno = EINVAL;
        return NULL;
    }

    pthread_mutex_lock(&self->lock);

    if(self->front == self->rear){
        self->rear = NULL;
    }
    self->front = self->front->next;
    sem_wait(&self->items);

    void* val = node->item;
    free(node);

    pthread_mutex_unlock(&self->lock);
    return val;
}
