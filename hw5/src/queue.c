#include <errno.h>
#include "queue.h"
#include <csapp.h>

//TODO @1178 Because you are implementing a blocking queue, all operations should not return until they are completed.
queue_t *create_queue(void) {
    //When initialised, the queue is empty
    struct queue_t* queue = (queue_t*) calloc(1, sizeof(queue_t));
    if(queue==NULL) return NULL;
    queue->invalid = false; //Just in case
    if(sem_init(&queue->items,0,0)!=0)  return NULL;                    // Number of items
    if(pthread_mutex_init(&queue->lock, NULL)!=0)   return NULL;        // The queue operation
    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    //https://computing.llnl.gov/tutorials/pthreads/
    if(destroy_function==NULL || self==NULL || self->invalid == true) {
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

    if( self==NULL || self->invalid == true || item==NULL ) {
        errno = EINVAL;
        return false;
    }

    queue_node_t* newnode = calloc(1, sizeof(queue_node_t));
    newnode->item = item;
    newnode->next = NULL;
    /*
     * Since we always have an item available, we dont P on number of items,
     * but rather 1. start directly by taking hold of the queue. We then do the
     * processing, the 2. announce that item is available, then 3. release the Q.
     */
    pthread_mutex_lock(&self->lock);

    if(self->front==NULL)
        self->front = newnode;

    if(self->rear !=NULL)
        self->rear->next = newnode;

    self->rear = newnode;
    V(&self->items);

    pthread_mutex_unlock(&self->lock);
    return true;
}

void *dequeue(queue_t *self) {


    if(self == NULL || self->invalid==true) {
        errno = EINVAL;
        return NULL;
    }
    /*
     * However, while dequeuing, we dont directly take control of the queue
     * unless an item is available. So 1. we first P on an item, after that
     * 2. we grab the control of the queue, 3. update the item count, then
     * 4. lose the control of the queue.
     */
    P(&self->items);             //Only if atleast an item is there.
    pthread_mutex_lock(&self->lock);

    queue_node_t *node = self->front;

    if(self->front == self->rear){
        self->rear = NULL;
    }
    self->front = self->front->next;

    void* val = node->item;
    free(node);

    pthread_mutex_unlock(&self->lock);
    return val;
}
