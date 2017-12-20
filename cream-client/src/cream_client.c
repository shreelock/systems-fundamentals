#include "client.h"
#include "cream.h"
#include "csapp.h"

int main(int argc, char **argv) {
    args_t *args;
    int exit_code;

    signal(SIGPIPE, SIG_IGN);

    if (!(args = parse_args(argc, argv))) {
        app_error("invalid number of arguments.\n");
    }

    exit_code = client_init(args);

    free(args->hostname);
    free(args->port);
    free(args);

    exit(exit_code);
}

args_t *parse_args(int argc, char **argv) {
    if (argc != 3) {
        return NULL;
    }

    args_t *args = Malloc(sizeof(args_t));
    args->hostname = strdup(argv[1]);
    args->port = strdup(argv[2]);

    return args;
}

int client_init(args_t *args) {
    char buf[MAX_BUF + 1];
    const char *prompt = ">";
    size_t prompt_len = strlen(prompt);

    while (true) {
        write(STDOUT_FILENO, prompt, prompt_len);

        memset(buf, 0, sizeof(buf));

        if (!fgets(buf, sizeof(buf), stdin)) {
            break;
        }

        buf[strlen(buf) - 1] = 0;

        if (handle_request(args->hostname, args->port, buf) < 0) {
            break;
        }
    }

    return 0;
}

int handle_request(char *hostname, char *port, char *input) {
    char *request;
    char *key;
    char *value;
    int fd1, fd2;

    if (!(request = strtok(input, " ")))
        goto bad;

    key = strtok(NULL, " ");
    value = strtok(NULL, " ");

    if (!strcmp(request, PUT_REQUEST)) {
        if (!key || !value) {
            goto bad;
        }

        return handle_put(Open_clientfd(hostname, port), key, value);
    } else if (!strcmp(request, GET_REQUEST)) {
        if (!key) {
            goto bad;
        }

        return handle_get(Open_clientfd(hostname, port), key);
    } else if (!strcmp(request, EVICT_REQUEST)) {
        if (!key) {
            goto bad;
        }

        return handle_evict(Open_clientfd(hostname, port), key);
    } else if (!strcmp(request, CLEAR_REQUEST)) {
        return handle_clear(Open_clientfd(hostname, port));
    } else if (!strcmp(request, TEST_REQUEST)) {
        if (!key || !value) {
            goto bad;
        }
        fd1 = Open_clientfd(hostname, port);
        fd2 = Open_clientfd(hostname, port);
    
        return handle_test(fd1, fd2, key, value);
    } else if (!strcmp(request, QUIT)) {
        return -1;
    }

bad:
    printf("%s: invalid request type or arguments.\n", request);
    fflush(stdout);
    return 0;
}

int handle_put(int clientfd, char *key, char *value) {
    request_header_t request_header = {PUT, strlen(key), strlen(value)};
    response_header_t response_header;

    Rio_writen(clientfd, &request_header, sizeof(request_header));
    Rio_writen(clientfd, key, request_header.key_size);
    Rio_writen(clientfd, value, request_header.value_size);

    Rio_readn(clientfd, &response_header, sizeof(response_header));

    close(clientfd);

    if (response_header.response_code == OK) {
        printf("put request completed successfully.\n");
    } else {
        printf(
            "put request failed with response code of %d, and key_size of %d\n",
            response_header.response_code, response_header.value_size);
    }

    return 0;
}

int handle_get(int clientfd, char *key) {
    request_header_t request_header = {GET, strlen(key), 0};
    response_header_t response_header;
    char *buf;

    Rio_writen(clientfd, &request_header, sizeof(request_header));
    Rio_writen(clientfd, key, request_header.key_size);

    Rio_readn(clientfd, &response_header, sizeof(response_header));

    if (response_header.response_code == OK &&
        response_header.value_size != 0) {
        buf = Calloc(1, response_header.value_size + 1);
        Rio_readn(clientfd, buf, response_header.value_size);
        printf("get request completed successfully.\n");
        printf("value: %s\n", buf);
        free(buf);
    } else {
        printf("get request failed with response code of %d, and key_size of "
               "%d.\n",
               response_header.response_code, response_header.value_size);
    }

    close(clientfd);

    return 0;
}

int handle_evict(int clientfd, char *key) {
    request_header_t request_header = {EVICT, strlen(key), 0};
    response_header_t response_header;

    Rio_writen(clientfd, &request_header, sizeof(request_header));
    Rio_writen(clientfd, key, request_header.key_size);

    Rio_readn(clientfd, &response_header, sizeof(response_header));

    close(clientfd);

    if (response_header.response_code == OK) {
        printf("evict request completed successfully.\n");
    } else {
        printf("evict request failed with response code of %d, and key_size of "
               "%d.\n",
               response_header.response_code, response_header.value_size);
    }

    return 0;
}

int handle_clear(int clientfd) {
    request_header_t request_header = {CLEAR, 0, 0};
    response_header_t response_header;

    Rio_writen(clientfd, &request_header, sizeof(request_header));

    Rio_readn(clientfd, &response_header, sizeof(response_header));

    close(clientfd);

    if (response_header.response_code == OK) {
        printf("clear request completed successfully.\n");
    } else {
        printf("clear request failed with response code of %d, and key_size of "
               "%d.\n",
               response_header.response_code, response_header.value_size);
    }

    return 0;
}

int handle_test(int putfd, int getfd, char *key, char *value) {
    handle_put(putfd, key, value);

    char *buf;
    int actual_value_size = strlen(value);
    request_header_t request_header = {GET, strlen(key), 0};
    response_header_t response_header;

    Rio_writen(getfd, &request_header, sizeof(request_header));
    Rio_writen(getfd, key, request_header.key_size);

    Rio_readn(getfd, &response_header, sizeof(response_header));

    if (response_header.response_code != OK) {
        printf("get request failed with response code of %d, and key_size of "
               "%d.\n",
               response_header.response_code, response_header.value_size);
        close(getfd);
        return 0;
    }

    buf = Calloc(1, response_header.value_size + 1);
    Rio_readn(getfd, buf, response_header.value_size);

    close(getfd);

    if (actual_value_size != response_header.value_size) {
        printf("Server returned incorrect number of bytes.\n");
        printf("Expected Size:%d\n", actual_value_size);
        printf("Actual Size:%d\n", response_header.value_size);
        printf("Expected Value:%s\n", value);
        printf("Actual Value:%s\n", buf);
    } else if (memcmp(value, buf, actual_value_size)) {
        printf("Server returned incorrect value.\n");
        printf("Expected Value:%s\n", value);
        printf("Actual Value:%s\n", buf);
    } else {
        printf("Successfully inserted and returned key/value pair.\n");
    }

    free(buf);

    return 0;
}
