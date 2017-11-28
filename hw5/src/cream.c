#include <queue.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <csapp.h>
#include "cream.h"
#include "utils.h"

void printhashmap(hashmap_t* hmap);

void *thread(void *queue) ;

void printqueue(queue_t* q) ;

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
void print_help(){
    char* string = "./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES\n"
            "-h                 Displays this help menu and returns EXIT_SUCCESS.\n"
            "NUM_WORKERS        The number of worker threads used to service requests.\n"
            "PORT_NUMBER        Port number to listen on for incoming connections.\n"
            "MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.";
    printf("%s\n", string);
}
/* Global Shared Queue */
queue_t *request_queue;

int main(int argc, char *argv[]) {
    long NUM_WORKERS=1;//, MAX_ENTRIES;
    char *PORT_NUMBER = 0;
    // ------------------------------------------------
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    // ------------------------------------------------

    if(argc!=4){
        print_help();
        return EXIT_SUCCESS;
    } else {
        if (strcmp(argv[0], "-h") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[2], "-h") == 0) {
            print_help();
            return EXIT_SUCCESS;
        }
        NUM_WORKERS = 0;//atol(argv[1]);
        PORT_NUMBER = argv[2];
        //MAX_ENTRIES = (long) argv[2];
    }
    //hashmap_t *hashmap = create_map((uint32_t) MAX_ENTRIES, jenkins_one_at_a_time_hash, (destructor_f) map_free_function2);

    listenfd = Open_listenfd(PORT_NUMBER);
    request_queue = create_queue();


    for(int i=0; i<NUM_WORKERS; i++)
        Pthread_create(&tid, NULL, thread, &request_queue);

    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen);
        enqueue(request_queue, (void*) (intptr_t) connfd);
        printf("Enqueued : %d\n", connfd);
        printqueue(request_queue);
    }


}

void *thread(void* queue) {
    queue_t* q = (queue_t*) queue;
    Pthread_detach(pthread_self());
    while(1) {
        int connfd = (int) (intptr_t) dequeue(q);
        printf("dequeued : %d\n", connfd);
        //waits until it can dequeue the element from the queue
        //Does the thing -
        request_header_t* req_header = (request_header_t*) malloc(sizeof(request_header_t));
        read(connfd, &req_header, sizeof(request_header_t));
//        printf(req_header->request_code);
        // 1. Serve the request
        // 2. send s response to the client
        // 3. close the connection
        Close(connfd);
    }
}

int hashmap_test(int argc, char *argv[]) {
    hashmap_t* hm = create_map(3, jenkins_hash2, map_free_function2);
    char* s1 = malloc(6*sizeof(char));    strcpy(s1,"key1");
    char* s2 = malloc(6*sizeof(char));    strcpy(s2,"key2");
    char* s3 = malloc(6*sizeof(char));    strcpy(s3,"key3");
    char* s4 = malloc(6*sizeof(char));    strcpy(s4,"key4");
    char* s5 = malloc(6*sizeof(char));    strcpy(s5,"key5");

    char* d1 = malloc(6*sizeof(char));    strcpy(d1,"val1");
    char* d2 = malloc(6*sizeof(char));    strcpy(d2,"val2");
    char* d3 = malloc(6*sizeof(char));    strcpy(d3,"val3");
    char* d4 = malloc(6*sizeof(char));    strcpy(d4,"val4");
    char* d5 = malloc(6*sizeof(char));    strcpy(d5,"val5");

    map_key_t mk1 = MAP_KEY(s1, 10);
    map_key_t mk2 = MAP_KEY(s2, 10);
    map_key_t mk3 = MAP_KEY(s3, 10);
    map_key_t mk4 = MAP_KEY(s4, 10);
    map_key_t mk5 = MAP_KEY(s5, 10);

    map_val_t mv1 = MAP_VAL(d1, 10);
    map_val_t mv2 = MAP_VAL(d2, 10);
    map_val_t mv3 = MAP_VAL(d3, 10);
    map_val_t mv4 = MAP_VAL(d4, 10);
    map_val_t mv5 = MAP_VAL(d5, 10);

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


    char* findkey = (char*) malloc(6*sizeof(char));    strcpy(findkey,"key1");    map_key_t fk = MAP_KEY(findkey, 10);
    printf("Getting : %s\n", (char *) fk.key_base);
    map_val_t mvt = get(hm, fk);
    printf("%s\n\n", (char*) mvt.val_base);

    put(hm, mk5, mv5, true);
    printf("Putting : %s\n", (char *) mk5.key_base);
    printhashmap(hm);

    printf("Clearing the map\n");
    clear_map(hm);
    printhashmap(hm);

    printf("Invalidating the map\n");
    invalidate_map(hm);
    printhashmap(hm);

    exit(0);
}

int queue_test(int argc, char *argv[]) {
    queue_t* gh = create_queue();
    char* f1 = "shreenathrules";
    int f2 = 90;
    void* ff = &f2;
    enqueue(gh, f1);
    enqueue(gh, ff);
//    enqueue(gh,"3");
//    enqueue(gh,"4");
//    printf("%s", (char *) dequeue(gh));
    printf("\n%s\n",  (char*) dequeue(gh));
    printf("\n%d\n", *(int*) dequeue(gh));
//    printf("%s", (char *) dequeue(gh));
//    printf("%s", (char *) dequeue(gh));
//    enqueue(gh,"5");
//    invalidate_queue(gh, queue_free_function1);
    exit(0);
}

void printhashmap(hashmap_t* hmap) {
    printf("size:%d, capacity:%d\n", hmap->size, hmap->capacity);
    for (int i=0;i<hmap->capacity;i++){
        map_node_t* n = hmap->nodes + i;
        printf("%s:%s\n", (char*) n->key.key_base, (char*) n->val.val_base);
    }
    printf("%s\n", strerror(errno));
    printf("\n");
}

void printqueue(queue_t* q) {
    queue_node_t *n = q->front;
    printf("Queue : \n");
    while(n!=NULL){
        printf("%d\n", (int) (intptr_t) n->item);
        n=n->next;
    }
    printf("\n");
}
