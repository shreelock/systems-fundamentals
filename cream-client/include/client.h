#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

#define MAX_BUF 1024

typedef struct args_t {
    char *hostname;
    char *port;
} args_t;

args_t *parse_args(int argc, char **argv);
int client_init(args_t *args);
int handle_request(char *hostname, char *port, char *input);
int handle_put(int clientfd, char *key, char *value);
int handle_get(int clientfd, char *key);
int handle_evict(int clientfd, char *key);
int handle_clear(int clientfd);
int handle_test(int putfd, int getfd, char *key, char *value);

const char *PUT_REQUEST = "put";
const char *GET_REQUEST = "get";
const char *EVICT_REQUEST = "evict";
const char *CLEAR_REQUEST = "clear";
const char *TEST_REQUEST = "test";
const char *QUIT = "quit";
#endif
