# Homework 5 - CSE 320 - Fall 2017
#### Professor Jennifer Wong-Ma & Professor Eugene Stark

### **Due Date: Friday 12/01/2017 @ 11:59pm**

## Updates

If updates are made to this document or the base code, they will be noted in this section.
Remember that base code changes require you to fetch and merge as described in `hw0`.
All updates to this document are prefaced with *UPDATE*.

### Base Code

* 11/13/17 - Changed `TTL` macro in `const.h` from 60 to 5
* 11/13/17 - Added `Makefile.config` and modified the `Makefile` to include it
* 11/14/17 - Updated `Makefile` to support `all`, `ec`, `debug`, and `debug_ec` targets

### Client
* 11/14/17 - Released cream client at https://gitlab02.cs.stonybrook.edu/cse320/cream-client

### Homework Document

* 11/13/17 - Added note about destroying nodes in the hash map when they are evicted if the map is full
* 11/14/17 - Updated information about cream client

## Introduction

The goal of this assignment is to become familiar with low-level POSIX threads, multi-threading safety, concurrency guarantees, and networking.
In part 1 of this assignment you will implement a concurrent queue using the producers/consumers locking pattern.
In part 2 you will implement a concurrent hash map using the readers/writers locking pattern.
In part 3 you will use the data structures implemented in the previous parts to build an in-memory, multi-threaded, caching server similar to [__Memcached__](https://memcached.org/).

### Takeaways

After completing this homework, you should:

* Understand thread execution, locks, and semaphores
* Have an advanced understanding of POSIX threads
* Insight into the design of concurrent data structures
* Have a basic understanding of socket programming
* Have enhanced your C programming abilities

## Hints and Tips

* We strongly recommend you check the return codes of all system calls. This will help you catch errors.
* **BEAT UP YOUR OWN CODE!** Throw lots of concurrent calls at your data structure libraries to ensure safety.
* Your code should **NEVER** crash. We will be deducting points for each time your program crashes during grading. Make sure your code handles invalid usage gracefully.
* You should make use of the macros in `debug.h`. You would never expect a library to print arbitrary statements as it could interfere with the program using the library. **FOLLOW THIS CONVENTION!** `make debug` is your friend.

> :scream: **DO NOT** modify `const.h`, `cream.h`, `hashmap.h`, `queue.h`, `utils.h`, and `utils.c`.

> :scream: **YOU MUST** use the structs, enums, and locks defined in `queue.h`, `hashmap.h`, and `cream.h` for your implementation.
> We will check these when we grade your program.

> :nerd: When writing your program, try to comment as much as possible and stay consistent with your formatting.
> In order for us to be able to help you, organize your code into modules.

## Helpful Resources

### Textbook Readings

You should make sure that you understand the material covered in chapters **11.4** and **12** of **Computer Systems: A Programmer's Perspective 3rd Edition** before starting this assignment.
These chapters cover networking and concurrency in great detail and will be an invaluable resource for this assignment.

### pthread Man Pages

The pthread man pages can be easily accessed through your terminal.
However, [this opengroup.org site](http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html) provides a list of all the available functions.
The same list is also available for [semaphores](http://pubs.opengroup.org/onlinepubs/7908799/xsh/semaphore.h.html).

## Getting Started

Fetch and merge the base code for `hw5` as described in `hw0`. You can find it at this link: https://gitlab02.cs.stonybrook.edu/cse320/hw5.
Remember to use the `--stategy-option theirs` flag for the `git merge` command to avoid merge conflicts in the Gitlab CI file.

Here is the structure of the base code:

```
repo
├── Makefile
├── Makefile.config
├── include
│   ├── const.h
│   ├── cream.h
│   ├── debug.h
│   ├── extracredit.h
│   ├── hashmap.h
│   ├── queue.h
│   └── utils.h
├── src
│   ├── cream.c
│   ├── extracredit.c
│   ├── hashmap.c
│   ├── queue.c
│   └── utils.c
└── tests
    ├── extracredit_tests.c
    ├── hashmap_tests.c
    └── queue_tests.c
```

## Part I: Concurrent Queue

A queue is a first-in first-out (FIFO) linear data structure where elements are inserted at one end (the rear), and are removed from the other end (the front).
In this part of the assignment, you will create a concurrent, blocking queue that follows the **producers/consumers** pattern.
The `queue_t` struct provided in `queue.h` includes a mutex and a semaphore that **YOU MUST** use to implement the producers/consumers pattern.

> :scream: **DO NOT** modify `queue.h`. It will be overwritten during grading.

Your queue will support two basic operations, `enqueue` and `dequeue`, which insert elements at the rear of the queue and remove elements from the front of the queue, respectively.
While a queue can typically be implemented with either an array or a linked list, you will implement the queue using a **singly-linked list**.
Implementing a queue using a linked list removes the need to resize the queue and  also allows `enqueue` and `dequeue` to be performed in constant time.

Your implementation of the queue **MUST** use the structs and function prototypes defined in `queue.h`.

> :nerd: To better understand the producers/consumers pattern, read section **12.5** in **Computer Systems: A Programmer's Perspective 3rd Edition**.

### Structures

In `queue.h` you will find the following `queue_node_t` and `queue_t` structs and a typedef for `item_destructor_f`, which is a function pointer.

- `queue_node_t` A single node in the queue.
    - `void *item` A pointer to the data stored by this node.
    - `struct queue_node_t *next` A pointer to the next node in the queue.
- `queue_t`
    - `queue_node_t *front, *rear` Pointers to the front and rear of the queue.
    - `sem_t items` A semaphore used to track the number of items in the queue.
    - `pthread_mutex_t lock` A mutex used to protect the queue when modifications are being made by a thread.
    - `bool invalid` A flag to indicate that the queue has been invalidated and should not be used.
- `item_destructor_f`
    - A typedef for a function pointer that takes `void *` and returns nothing
    - A function of this type will be used to clean up every item in the queue when the queue is destroyed

**YOU MUST** use `items` and `lock` to implement the producers/consumers pattern.

### Operations

For all operations defined below, function parameters are invalid if any of the following are true:
- Any pointers passed to the function are `NULL`.
- The `queue_t` pointer passed to the function has been invalidated.

You should implement the following functions to create and manage an instance of `queue_t`:

- `queue_t *create_queue(void);`
    - This function will `calloc(3)` a new instance of `queue_t` and initialize all locks and semaphores in the `queue_t` struct.
    - *Returns:* A valid pointer to an initialized `queue_t` instance or `NULL`.
    - *Error case:* If `calloc(3)` returns `NULL` or the locks fail to initialize, return `NULL`.

- `bool invalidate_queue(queue_t *self, item_destructor_f destroy_function);`
    - This function will invalidate the `queue_t` instance pointed to by `self`.
    - It will call `destroy_function` on all remaining items in the queue and `free(3)` the `queue_node_t` instances.
    - It will set the `invalid` flag in `self` to true to indicate that the queue is not usable.
    - It **must not** free `self` or destroy any locks in `self`.
    - *Returns:* `true` if the invalidation was successful, `false` otherwise
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `false`.

- `bool enqueue(queue_t *self, void *item);`
    - This function should `calloc(3)` a new `queue_node_t` instance to add to the queue.
    - **Note:** You should assume `item` is a valid pointer. The new `queue_node_t` instance should have a pointer to the original `item`. **DO NOT** `malloc` new space for `item`.
    - *Returns:* `true` if the operation was successful, `false` otherwise.
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `false`.

- `void *dequeue(queue_t *self);`
    - Removes the item at the front of the queue pointed to by `self`.
    - This function should `free(3)` the `queue_node_t` instance being removed from the queue.
    - This function should block until an item is available to dequeue.
    - **Note:** Because you did not `malloc` space for the item being removed, **DO NOT** `free` the item before returning it.
    - *Returns:* A pointer to the item stored at the front of the queue.
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `NULL`.

**Note:** Because you are implementing a blocking queue, all operations should not return until they are completed.
Note that this does not apply for cases with invalid arguments.
In these cases, `errno` should be set and the function should return immediately.

### Making it Safe

All of the functions must be MT-Safe (multi-threading safe).
Any number of threads should be able to call these functions without any data corruption.
Choosing where and when to lock in the queue functions is up to you.

You should consider that `enqueue`, `dequeue` and `invalidate_queue` all modify the queue data structure.
When implementing these functions, consider what happens if operations are interleaved.

## Part II: Concurrent Hash map

A hash map is a generic data structure that allows for insertion, searching, and deletion in expected constant time.
Hash maps store key-value pairs.
Each key must be unique, but values can be repeated.
You will implement an [__open-addressed hash map__](http://www.algolist.net/Data_structures/Hash_table/Open_addressing) backed by an array that uses linear probing to deal with collisions.
Your map will support the `put`, `get`, and `remove` operations.

Hash maps compute the index for a given key using the formula `index = hash(key) % table_capacity`, where `hash()` is a hashing function that returns an unsigned integer.
To insert, the map tries to put the key/value pair at the computed index.
Linear probing is used to deal with hash collisions (i.e. when two or more keys hash to the same index).
In this case, the map will search larger indexes sequentially, wrapping around the array if necessary, until an empty slot is found.
The new entry will be inserted at this empty slot.

Searching for a value given a key is similar.
First, the starting index is computed using the previous formula.
The map starts looking for the key at the computed index and continues searching sequentially through the map until the key is found, it gets back to the original index, or an empty slot is found.
In the latter two situations, the map will conclude that the key is not present.

Deletions can be tricky.
It is incorrect to just remove the key/value pair from the array, leaving behind an empty slot.
To see why, consider the following put/get calls:

```
// assume key1, key2, and key3 are keys that map to index 0
// assume the map has a capacity of 5
map.put(key1, value1)
map.put(key2, value2)
map.put(key3, value3)
map.remove(key2)
value3 = map.get(key3)
```

After the `put` statements, the array is: `key1 key2 key3 EMPTY EMPTY`.
After the `remove`, this becomes: `key1 EMPTY key3 EMPTY EMPTY`.
When the `get` statement is executed, the map will start searching for `key3` at index 0, see the `EMPTY` slot at index 1, and conclude that `key3` doesn't exist.
This is incorrect as `key3` can be found at index 2 of the array.

To fix this, the map sets a special tombstone flag at every deleted index.
When searching, the map can skip over a tombstone and continue searching at the next index.
When inserting, the map can treat the tombstone as an empty slot and insert a new key-value pair.

### Structures

In `hashmap.h`, you will find declarations for `hashmap_t`, `map_node_t`, `map_key_t`, `map_val_t`, `map_node_t`, `hash_func_f`, and `destructor_f`.

> :scream: **DO NOT** modify `hashmap.h`. It will be overwritten for grading.

Each of the fields in the `hashmap_t` struct help manage one instance of a hash map.
The fields are defined as follows:

- `uint32_t capacity` The maximum number of items that the map can hold.
- `uint32_t size` The number of items currently stored in the map.
- `map_node_t *nodes` The base address of the array of map nodes.
- `hash_func_t hash_function` The hash function that the map uses to hash keys.
- `destructor_f destroy_function` The destructor function that the map uses to free keys and values when it is destroyed.
- `int num_readers` The number of readers accessing the map.
- `pthread_mutex_t write_lock` A mutex used to lock the entire map for writing.
- `pthread_mutex_t fields_lock` A mutex used to protect the fields in the struct.
- `bool invalid` A flag to indicate that the map has been invalidated and should not be used.

**YOU MUST** use the `num_readers`, `write_lock`, and `fields_lock` fields to implement the readers/writers pattern, as discussed in **Section 12.5 (Page 1006)** of your textbook.

The map holds an array of `map_node_t` instances.
The `map_node_t` struct has the following fields:

- `map_key_t key` The key associated with the node
- `map_val_t val` The value associated with the node
- `bool tombstone` True if the node is a tombstone

`map_key_t` and `map_val_t` are wrapper structs that contain a base pointer and a length (in bytes) of keys and values, respectively.
Note that storing the length of the key and value in each node allows different data types to be stored in the same map.

`hash_func_f` and `destructor_f` are typedefs for function pointers.
A `hash_func_f` function takes a `map_key_t` instance and returns a `uint32_t`.
It will be used to hash keys when inserting, searching, and deleting.
A `destructor_f` function takes a `map_key_t` and `map_val_t` instance and does not return anything.
A function of this type will be used to clean up keys and values when the map is destroyed.

The `get_index` function in `utils.c` uses the capacity of the map and the hash of a key to retrieve the index that a key will hash to.

Here is a diagram to help you visualize the `hashmap_t` struct:

![hashmap_diagram](diagrams/hashmap_diagram.png)

### Operations

For all operations defined below, function parameters are invalid if any of the following are true:
- Any pointers passed to the function are `NULL`.
- For `map_key_t` and `map_val_t` instances, the base pointer is `NULL` or the size is 0.
- The `hashmap_t` pointer passed to the function has been invalidated.

You should implement the following operations to create and manage a hash map:

- `hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function)`
    - This will `calloc(3)` a new instance of `hashmap_t` that manages an array of `capacity` `map_node_t` instances.
    - *Returns:*  A valid pointer to a `hashmap_t` instance, or `NULL`.
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `NULL`.
    - *Error case:* If `calloc(3)` is unsuccessful or any of the locks cannot be initialized, return `NULL`.

- `bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force)`
    - This will insert a key/value pair into the hash map pointed to by `self`.
    - If the key already exists in the map, update the value associated with it and return `true`.
    - If the map is full and `force` is `true`, overwrite the entry at the index given by `get_index` and return `true`.
    - *Returns:* `true` if the operation was successful, `false` otherwise.
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `false`.
    - *Error case:* If the map is full and `force` is false, set `errno` to `ENOMEM` and return `false`.
    - **Note:** The `force` flag should be ignored if the map is not full.
    - **Note:** If the pointers in `key` and `val` are not `NULL`, you should assume they are valid and should not `malloc(3)` new space for them.
    - **UPDATE:** If a node is evicted from the map to fulfill this request, the `destroy_function` in `self` should be called on the evicted node to properly destroy it.

- `map_val_t get(hashmap_t *self, map_key_t key)`
    - Retrieves the `map_val_t` corresponding to `key`.
    - *Returns:* The corresponding value.
      If `key` is not found in the map, the `map_val_t` instance will contain a `NULL` pointer and a `val_len` of 0.
    - *Error case:* If any of the parameters are invalid, set `errno` to `EINVAL`.
    The returned `map_val_t` instance should contain the same fields as the case where `key` is not found.

- `map_node_t delete(hashmap_t *self, map_key_t key)`
    - Removes the entry with key `key`.
    - *Returns:* The removed `map_node_t` instance
    - *Error case:* If any of the parameters are invalid, set `errno` to `EINVAL` and return a `map_node_t` with all pointers set to `NULL` and lengths set to 0.
    - **Note:** Because you did not `malloc(3)` new space for the pointer in the returned value, you should not `free(3)` it before returning.

- `bool clear_map(hashmap_t *self)`
    - Clears all remaining entries in the map.
    - It will call the `destroy_function` in `self` on every remaining item.
    - It **must not** free any pointers or destroy any locks in `self`.
    - *Returns:* `true` if the operation was successful, `false` otherwise.
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `false`.

- `bool invalidate_map(hashmap_t *self)`
    - This will invalidate the `hashmap_t` instance pointed to by `self`.
    - It will call the `destroy_function` in `self` on every remaining item.
    - It will `free(3)` the `nodes` pointer in `self`.
    - It will set the `invalid` flag in `self` to `true`.
    - It **must not** free `self` or destroy any locks in `self`.
    - *Returns:* `true` if the invalidation was successful, `false` otherwise.
    - *Error case:* If any parameters are invalid, set `errno` to `EINVAL` and return `false`.

### Making it Safe

All operations on the hash map must be multi-threading safe.
This will allow multiple threads to access the map concurrently without data corruption.
Using the locks and `num_readers` variable in the `hashmap_t` struct, you must implement the readers/writers pattern for the map.

## Part 3: C.R.E.A.M.

In computing, a cache is a high-speed data storage layer which stores a subset of transient data that an application requests such that future requests for that data are served faster than normal.
Caching allows applications to efficiently reuse previously retrieved or computed data by trading storage space for speed.
Caching can be leveraged to improve performance in a variety of applications such as operating systems, DNS, web applications, and databases.
As a result of caching's numerous benefits, many general purpose caching services such as [**Memcached**](https://github.com/memcached/memcached/wiki) and [**Redis**](https://redis.io/) have been developed.
These caching services can be used by any application and provide a ton of flexibility to the programmers using them.

In this part of the assignment you will use the concurrent data structures implemented in the previous parts of this assignment to build the server for the general purpose in-memory key-value caching service **Cache Rules Everything Around Me** ([C.R.E.A.M.](https://www.youtube.com/watch?v=PBwAxmrE194) or `cream` for short).

> :thinking: You are only responsible for implementing a server for `cream`.
> A client that can be used for testing and debugging your server is available on Gitlab at https://gitlab02.cs.stonybrook.edu/cse320/cream-client.

### C.R.E.A.M. Overview

```
./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES
-h                 Displays this help menu and returns EXIT_SUCCESS.
NUM_WORKERS        The number of worker threads used to service requests.
PORT_NUMBER        Port number to listen on for incoming connections.
MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.
```

`cream` will service a request for any client that connects to it.
`cream` will service **ONE** request per connection, and will terminate any connection after it has fulfilled and replied to its request.

![Server Diagram](./diagrams/server.png)

The diagram above shows how `cream` handles client requests.
On startup `cream` will spawn `NUM_WORKERS` worker threads for the lifetime of the program, bind a socket to the port specified by `PORT_NUMBER`, and infinitely listen on the bound socket for incoming connections.
Clients will attempt to establish a connection with `cream` which will be accepted in `cream`'s main thread.
After accepting the client's connection `cream`'s main thread adds the accepted socket to a **request queue** so that a blocked worker thread is unblocked to service the client's request.
Once the worker thread has serviced the request it will send a response to the client, close the connection, and block until it has to service another request.

> :warning: `cream`'s main thread **MUST NOT** read from the accepted socket.
> If the thread accepting connections in a server is blocked while reading from a socket, incoming connections will be queued in the lower protocol layer and potentially lost.

> :warning: After initialization `cream` **MUST** have exactly `NUM_WORKERS` worker threads running at all times.

All client requests access `cream`'s **underlying data store** in some way.
This underlying data store is what `cream` uses to cache values sent to it by clients.
The different types of requests and the structure of the requests that `cream` services are determined by the protocol that `cream` adheres to.
You **MUST** implement `cream` such that it adheres to the **Cache Money Protocol** (CMP) described in the next section of this document.

> :thinking: The **request queue** should be an instance of `queue_t`.
> The **underlying data store** should be an instance of `hashmap_t` with capacity `MAX_ENTRIES`.

### Cache Money Protocol (CMP)

Before we can talk about the types of requests that `cream` handles, we must talk about the protocol it uses to communicate with clients.
The concept of a protocol is an important one to understand.
When implementing the protocol you need to follow the requirements and description **TO THE BYTE**.
The idea is to create a standard so that any program implementing a protocol will be able to connect and operate with any other program implementing the same protocol.
Any client should work with any server if they both implement the same protocol correctly.

CMP is implemented over a streaming socket and uses the request-response pattern.
It is a very basic protocol as the client will send only one message (a request) to the server, and the server will send only one message (a response) back to the client.
Both messages (request and response) have a unique header which must be prepended to the message's body.
The fields in the request and response headers are used to denote the type and content of the message being sent.
Without the use of a message header, the receiver of the message has no idea what it is receiving.

> :nerd: Request–response is a message exchange pattern in which a requester sends a  message to a responder system which receives, processes, and responds to the request.
> This is a simple, but powerful messaging pattern which allows two applications to have a conversation with one another over a connection.

#### Request Header

In order to assist with this assignment we have provided you with the following `request_header_t` struct located in `cream.h`.

```C
typedef struct request_header_t {
    uint8_t request_code;
    uint32_t key_size;
    uint32_t value_size;
} __attribute__((packed)) request_header_t;
```
We have also defined a `request_codes` enum in `cream.h` consisting of all requests and their corresponding `request_code`.

```C
typedef enum request_codes {
    PUT = 0x01,
    GET = 0x02,
    EVICT = 0x04,
    CLEAR = 0x08
} request_codes;
```

When reading from the client's connected socket the server should first read and examine the `request_header`.
The server will be able to determine what the client's request is based on the `request_code`, and the structure of the payload based on the `key_size` and `value_size`.
The server should check if the client's request is valid by examining the `key_size` and `value_size` fields, and checking if they fall within the appropriate ranges.
In order to help with this check we have defined the `MIN_KEY_SIZE`, `MAX_KEY_SIZE`, `MIN_VALUE_SIZE`, and `MAX_VALUE_SIZE` macros in `cream.h`.

> :warning: Not every request will need the `key_size` and `value_size` fields.
> However, the server should always assume that the received message begins with a header formatted exactly the same as the `request_header_t` struct.
> Any fields that are not needed will be zeroed out by the client.

#### Response Header

In order to assist with this assignment we have provided you with the following `response_header_t` struct located in `cream.h`.

```C
typedef struct response_header_t {
    uint32_t response_code;
    uint32_t value_size;
} __attribute__((packed)) response_header_t;
```
We have also defined a `response_codes` enum in `cream.h` consisting of all responses and their corresponding `response_code`.

```C
typedef enum response_codes {
    OK = 200,
    UNSUPPORTED = 220,
    BAD_REQUEST = 400,
    NOT_FOUND = 404
} response_codes;
```

When sending a response to the client, the server should make sure that the `response_header` is at the beginning of the message.
The client will be able to determine what the server's response is based on the `response_code`, and the structure of the payload based on the `value_size` field.

> :warning: Not every response will need the `value_size` field.
> However, the server **MUST** always send a message that begins with a header formatted exactly the same as the `response_header_t` struct.
> Any fields that are not needed **MUST** be zeroed out.

#### Hash Function

In order to assist with this assignment we have provided you with the `jenkins_one_at_a_time_hash` hash function located in `utils.c`.

```C
uint32_t jenkins_one_at_a_time_hash(map_key_t map_key) {
    const uint8_t* key = map_key.key_base;
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
```

This hash function **MUST** be used by the server to hash keys when inserting, retrieving, and deleting values from the underlying data store.

> :warning: If your server does not use this hash function you will lose **A LOT** of points when we test your program.

#### Put Request

When a client wants to insert a value into the cache it will connect to the server and send a request message with a `request_code` of `PUT`.
The server checks if the `key_size` and `value_size` fields in the `request_header` are valid, and then attempts to read the key and value from the connected socket.

For a `PUT` request the first `key_size` bytes after the `request_header` correspond to the key, and the the next `value_size` bytes after the key correspond to the value.

If the cache is full, the `PUT` operation will evict a value from the cache by overwriting the entry in the hash map at the index that the key hashes to.

After the `PUT` operation has completed the server will send a response message back to the client informing them of the status of their request.
The `response_code` in the header of the response message will be set to `OK` if the operation was completed successfully, or `BAD_REQUEST` if an error occurred, and `value_size` will be set to 0.

> :nerd: It may not be obvious, but your cache inherently uses a random **cache replacement policy**.
> A cache replacement policy is the algorithm that a cache uses to discard entries when it needs to make room for new ones.

#### Get Request

When a client wants to retrieve a value from the cache it will connect to the server and send a request message with a `request_code` of `GET`.
The server checks if the `key_size` field in the `request_header` is valid, and then attempts to read the key from the connected socket.

For a `GET` request the first `key_size` bytes after the `request_header` correspond to the key.

If the value that the client requested exists, the server will send a response message back to the client containing the corresponding value.
In the message, the `response_code` will be set to `OK`, the `value_size` will be set to the size of the value in bytes, and the first `value_size` bytes after the header will be the corresponding value.

If the value that the client requested does not exist, the server will still send a response message back to client.
In this scenario the `response_code` will be set to `NOT_FOUND`, and `value_size` will be set to 0.

#### Evict Request

When a client wants to delete a value from the cache it will connect to the server and send a request message with a `request_code` of `EVICT`.
The server checks if the `key_size` field in the `request_header` is valid, and then attempts to read the key from the connected socket.

Once the `EVICT` operation has completed, regardless of the outcome of this operation, the server will send a response message back to the client with a `response_code` of `OK` and `value_size` of 0.

> :thinking: While this may seem deceptive from the client's point of view, can you think of a reason why `EVICT` requests are handled like this?
> Is it possible for a value to be evicted without a clients request?

#### Clear Request

When a client wants to clear all values from the cache it will connect to the server and send a request message with a request code of `CLEAR`.

Once the `CLEAR` operation has completed the server will send a response message back to the client with a `response_code` of `OK` and `value_size` of 0.

#### Invalid Request

If a client sends a message to the server, and the `request_code` is not set to any of the values in the `request_codes` enum, the server will send a response message back to the client with a `response_code` of `UNSUPPORTED` and `value_size` of 0.

### Things to be aware of

Aside from issues that are discussed in the protocol your program should handle external errors such as connections getting closed, client programs getting killed, and a blocking syscall being interrupted.

Your program must handle:
* EPIPE
* SIGPIPE
* EINTR

> :warning: You are responsible for handling all types of errors that can occur.
> Read the textbook and man pages to find out what these errors are and how you can handle them.

## Extra Credit

### **Letting Us Know About Your EC**
**In your repo you will find a Makefile.config file.
Lines 1 and 2 contain commented out variables for each extra credit part of this assignment.
You must identify which parts of the extra credit you attempt.
You can do this by removing the `#` at the beginning of the line that corresponds to the extra credit you completed.**

> :scream: **YOU MUST** uncomment (delete the `#`) the variables that are relevant to the extra credit you've attempted.
> There will be no exceptions made for failing to do so.
> Furthermore, each individual extra credit section is **all or nothing**.
> It either works completely correctly during grading, or you will not receive points for that particular extra credit.

There are two extra credit opportunities.
Both involve changing your hash map implementation.
Because you cannot modify `hashmap.h`, you **MUST** make modifications in `extracredit.h` and implement the new map in `extracredit.c`.
You can add any necessary fields to the structs in `extracredit.h`, but the actual struct names and function prototypes **MUST NOT** be changed.
This is to ensure that your `CREAM` implementation can switch between the two maps.
To test your extra credit, compile with `make clean ec`.
This will link `extracredit.o` in the binary rather than `hashmap.o`.

### LRU Cache

Your current CREAM implementation removes a random key if the underlying hash map is full.
This may violate [temporal locality](https://en.wikipedia.org/wiki/Locality_of_reference).
Change your hash map implementation to use a Least Recently Used (LRU) replacement policy.
When the map is full and a `put` call is made with the `force` parameter set to `true`, the least recently used node should be overwritten.
If the map is full and `force` is set to `false`, set `errno` to `ENOMEM` and return `false` as usual.
Both `put` and `get` operations should count towards a node being recently used.
Consider the following example with a map of capacity 3:

```
// assume self is an initialized hashmap_t pointer to an empty hash map
put(self, k1, v1, true)   // k1/v1 becomes the LRU node
put(self, k2, v2, true)   
put(self, k3, v3, true)   
get(self, k1)             // k2/v2 becomes the LRU node

put(self, k4, v4, true)   // k2/v2 is overwritten so k3/v3 becomes the LRU node
put(self, k5, v5 false)   // does not modify map or the LRU node, sets errno to ENOMEM, and returns false
```

The `put` request for `k4/v4` will overwrite the pair `k2/v2` in the map.
To avoid race conditions, the LRU replacement must be done by the map, not by CREAM.

### TTL Eviction

Modern caching daemons support time to live (TTL) eviction.
This means that a key/value pair in the map will be removed after a set amount of time.
For this extra credit, modify the `extracredit.h` and `extracredit.c` files to support TTL eviction.
When an element is inserted, it should be valid for the number of seconds specified by the `TTL` macro in `const.h`.
Note that the map does not have to evict nodes as soon as the TTL expiration time occurs.
It can lazily remove nodes when `get` requests are made for them.
Also, the map can treat expired nodes as tombstones when doing linear probing.

## Submission Instructions

Make sure your hw5 directory looks like this and that your homework compiles:

```
repo
├── Makefile
├── Makefile.config
├── include
│   ├── const.h
│   ├── cream.h
│   ├── debug.h
│   ├── extracredit.h
│   ├── hashmap.h
│   ├── queue.h
│   └── utils.h
├── src
│   ├── cream.c
│   ├── extracredit.c
│   ├── hashmap.c
│   ├── queue.c
│   └── utils.c
└── tests
    ├── extracredit_tests.c
    ├── hashmap_tests.c
    └── queue_tests.c
```

To submit, run `git submit hw5`.
