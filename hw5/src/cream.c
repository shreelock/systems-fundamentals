#include <queue.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <csapp.h>
#include <extracredit.h>
#include "cream.h"
#include "utils.h"

void printhashmap(hashmap_t* hmap);

void *thread(void *vargp) ;

void printqueue(queue_t* q) ;

void do_the_thing(int connfd);

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
hashmap_t *global_hashmap;

int main(int argc, char *argv[]) {
    long NUM_WORKERS, MAX_ENTRIES;
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
        NUM_WORKERS = atol(argv[1]);
        PORT_NUMBER = argv[2];
        MAX_ENTRIES = atol(argv[3]);
    }

    global_hashmap = create_map((uint32_t) MAX_ENTRIES, jenkins_one_at_a_time_hash, (destructor_f) map_free_function2);

    listenfd = Open_listenfd(PORT_NUMBER);
    request_queue = create_queue();


    for(int i=0; i<NUM_WORKERS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen);
        enqueue(request_queue, (void*) (intptr_t) connfd);
        printf("New Client enqueued on fd:%d\n", connfd);
//        printqueue(request_queue);
    }


}

void *thread(void* vargp) {
    Pthread_detach(pthread_self());
    while(1) {
        int connfd = (int) (intptr_t) dequeue(request_queue);
        do_the_thing(connfd);
        Close(connfd);
    }
}

void do_the_thing(int connfd){

    //Init-ing request related values
    request_header_t* rq_header = (request_header_t*) calloc(sizeof(request_header_t), 1);
    size_t rq_header_size = sizeof(request_header_t);

    //Init-ing response related values
    response_header_t* rs_header = (response_header_t*) calloc(sizeof(response_header_t), 1 );
    size_t rs_header_size = sizeof(response_header_t);
    rs_header->value_size = 0x0;

    //Reading from the file descriptor
    Rio_readn(connfd, rq_header, rq_header_size);

    //Eextracting values from the Request Header
    uint8_t rq_code = rq_header->request_code;
    uint32_t rq_key_size = rq_header->key_size;
    uint32_t rq_val_size = rq_header->value_size;

    char* rq_key = (char*) calloc(rq_key_size, 1);
    char* rq_val = (char*) calloc(rq_val_size, 1);

    map_key_t key;
    map_val_t val;

    switch(rq_code) {
        case (0x01):
            // PUT request
            // key and value size check
            if(!(rq_key_size<=MAX_KEY_SIZE
                 && rq_key_size>=MIN_KEY_SIZE)
               || !(rq_val_size<=MAX_VALUE_SIZE
                    && rq_val_size>=MIN_VALUE_SIZE)) {

                rs_header->response_code = BAD_REQUEST;
                write(connfd, rs_header, rs_header_size);
                return;
            }
            // Read key and value from fd using key and value sizes
            Rio_readn(connfd, rq_key, rq_key_size);
            Rio_readn(connfd, rq_val, rq_val_size);

            // create key and values
            key = MAP_KEY(rq_key, rq_key_size);
            val = MAP_VAL(rq_val, rq_val_size);

            // do the operations
            bool put_result = put(global_hashmap, key, val, true);
            // ^^ we are using force=true, because we want to evict the key if there's no space
            printhashmap(global_hashmap);

            // output stuff
            if(put_result) {
                rs_header->response_code = OK;
                Rio_writen(connfd, rs_header, rs_header_size);
                return;
            }
            break;
        case (0x02):
            // GET request
            // Key check
            if(!(rq_key_size<=MAX_KEY_SIZE
                 && rq_key_size>=MIN_KEY_SIZE)) {
                rs_header->response_code = BAD_REQUEST;
                write(connfd, rs_header, rs_header_size);
                return;
            }

            // Read Key
            Rio_readn(connfd, rq_key, rq_key_size);

            // Create Key
            key = MAP_KEY(rq_key, rq_key_size);

            // Do operation
            val = get(global_hashmap, key);
            printhashmap(global_hashmap);

            // Output stuff
            if(val.val_base != NULL) {
                rs_header->response_code = OK;
                rs_header->value_size = (uint32_t) val.val_len;
                Rio_writen(connfd, rs_header, rs_header_size);
                Rio_writen(connfd, val.val_base, sizeof(val.val_base));
                return;
            } else {
                rs_header->response_code = NOT_FOUND;
                Rio_writen(connfd, rs_header, rs_header_size);
            }
            break;
        case (0x04):
            // EVICT request
            // Only Key check
            if(!(rq_key_size<=MAX_KEY_SIZE
                 && rq_key_size>=MIN_KEY_SIZE)) {
                rs_header->response_code = BAD_REQUEST;
                write(connfd, rs_header, rs_header_size);
                return;
            }

            // Read key
            Rio_readn(connfd, rq_key, rq_key_size);

            // Create Key
            key = MAP_KEY(rq_key, rq_key_size);

            // Do operation
            delete(global_hashmap, key);
            printhashmap(global_hashmap);

            // write outputs
            // Send OK at all steps
            rs_header->response_code = OK;
            Rio_writen(connfd, rs_header, rs_header_size);
            break;
        case (0x08):
            //CLEAR request
            // No key and value size check
            // do the operations
            clear_map(global_hashmap);
            printhashmap(global_hashmap);

            // output stuff
            rs_header->response_code = OK;
            Rio_writen(connfd, rs_header, rs_header_size);
            break;
        default:
            //Bad Request
            rs_header->response_code = UNSUPPORTED;
            Rio_writen(connfd, rs_header, rs_header_size);
            return;
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

void printhashmap(hashmap_t* hmap) {
    printf("size:%d, capacity:%d\n", hmap->size, hmap->capacity);
    for (int i=0;i<hmap->capacity;i++){
        map_node_t* n = hmap->nodes + i;
        printf("%s:%s:%ld\n", (char*) n->key.key_base, (char*) n->val.val_base, n->age);
    }
    printf("%s\n", strerror(errno));
    printf("\n");
}

void printqueue(queue_t* q) {
    pthread_mutex_lock(&q->lock);
    queue_node_t *n = q->front;
    printf("Queue : \n");
    while(n!=NULL){
        printf("%d\n", (int) (intptr_t) n->item);
        n=n->next;
    }
    printf("\n");
    pthread_mutex_unlock(&q->lock);
}