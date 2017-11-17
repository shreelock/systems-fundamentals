#include <queue.h>
#include <stdio.h>
#include "cream.h"
#include "utils.h"
void queue_free_function1(void *item) {
    free(item);
}

int main(int argc, char *argv[]) {
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
