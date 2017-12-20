# Homework 3 Dynamic Memory Allocator - CSE 320 - Fall 2017
#### Professor Jennifer Wong-Ma & Professor Eugene Stark

### **Due Date: Friday 10/13/2017 @ 11:59pm**


We **HIGHLY** suggest that you read this entire document, the book chapter,
and examine the base code prior to beginning. If you do not read the entire
document before beginning, you may find yourself doing extra work.

> :scream: Start early so that you have an adequate amount of time to test
your program!

> :scream: The functions `malloc`, `free`, `realloc`, `memalign`, `calloc`,
> etc., are **NOT ALLOWED** in your implementation. If any of these functions,
> or any other function with similar functionality is found in your program,
> you **will receive a <font color="red">ZERO</font>**.

**NOTE:** In this document, we refer to a word as 2 bytes (16 bits) and a memory
row as 4 words (64 bits). We consider a page of memory to be 4096 bytes (4 KB)

# Introduction

You must read **Chapter 9.9 Dynamic Memory Allocation Page 839** before
starting this assignment. This chapter contains all the theoretical
information needed to complete this assignment. Since the textbook has
sufficient information about the different design strategies and
implementation details of an allocator, this document will not cover this
information. Instead, it will refer you to the necessary sections and pages in
the textbook.

## Takeaways

After completing this assignment, you will have a better understanding of:
* The inner workings of a dynamic memory allocator
* Memory padding and alignment
* Structs and linked lists in C
* [ERRNO](https://linux.die.net/man/3/errno) numbers in C
* Unit testing in C

# Overview

You will create a segregated free list allocator for the x86-64 architecture
that manages up to 4 pages of memory at a time with the following features:

- Four explicit free lists
- A first-fit placement policy
- Immediate coalescing on free with blocks at higher memory addresses
    - Note: You will coalesce with lower memory addresses when requesting more
memory from `sf_sbrk`. This is the only case.
- Block splitting without creating splinters.

Free blocks will be inserted into the proper free list in **last in first out
(LIFO) order**.

You will implement your own versions of the **malloc**, **realloc**, and
**free** functions.

You will use existing Criterion unit tests and write your own to help debug
your implementation.

## Segregated Free List Management Policy

You **MUST** use a **segregated free list** as described in **Chapter 9.9.14
Page 863** to manage free blocks. Your segregated list will consist of four
free lists. The sizes of the blocks in the free lists are decided by
the max size macros defined in `include/sfmm.h`.

**NOTE:** We will replace `include/sfmm.h` with different constants. Do not modify
this file.

* List 1 will have blocks of sizes [32, `LIST_1_MAX`]
* List 2 will have blocks of sizes [`LIST_1_MAX` + 1, `LIST_2_MAX`]
* List 3 will have blocks of sizes [`LIST_2_MAX` + 1, `LIST_3_MAX`]
* List 4 will have blocks of sizes [`LIST_3_MAX` + 1, `LIST_4_MAX`]

**Note:** `LIST_4_MAX` is defined as `-1` in `include/sfmm.h`. This is because
there is no actual maximum for the final list. It holds all blocks larger than
`LIST_3_MAX`.

> :thinking: Why is the minimum block size 32? Think about how the header,
> footer, and alignment affect the minimum block size.

Within each free list, the blocks must be arranged in LIFO order. This means
that a block is always inserted at the beginning of a free list.

## Block Placement Policy

When allocating memory, use **first fit placement** to select the first block in
the appropriate sized list (more details in **Chapter 9.9.7 Page 849**).

If you do not find a block that fits in its list, continue searching the larger
lists.

## Immediate Coalescing

Your implementation must perform **immediate coalescing** with blocks of
**higher memory addresses** on free. This is similar to the procedure
described in **Chapter 9.9.10 Page 850**, but we only consider cases in which
the "next" block is free. Coalescing must **only** be done when `sf_free` is
called, for some cases for `sf_realloc` described below, and when `sf_sbrk` is
called. For the `sf_sbrk` case, you will coalesce with blocks at **lower
memory addresses** (this is described later).


## Splitting Blocks & Splinters

Your allocator must split blocks to reduce the amount of internal fragmentation.
Details about this feature can be found in **Chapter 9.9.8 Page 849**.

When splitting a block, your allocator **MUST NOT** create splinters.
Splinters are small blocks (< 32 bytes in our case) left after splitting a
free block. To avoid this, your allocator must "over allocate" the amount of
memory requested so that these splinters are not inserted into your free
lists. If splitting a block would create a splinter, leave the splinter as
part of the block. **DO NOT** attempt to coalesce the splinter with the block
after in memory.

## Block Headers & Footers

In **Chapter 9.9.6 Page 847 Figure 9.35**, the header is defined as 2 words (32
bits) to hold the block size and allocated bit. In this assignment, the header
will be 4 words (i.e. 64 bits or 1 memory row). The header fields will be
similar to those in the textbook but you will keep an extra field for recording
whether or not the block was padded.

Padding occurs when the allocated payload size is greater than the requested
size. Note that this includes splinters and extra memory for alignment.

**Block Header Format:**
<pre>
+--------------------------------------------+------------------+-------+-------+---------+
|                 unused                     |   block_size     |       | padded|  alloc  |
|                                            |                  |  00   |   1   |    1    |
|                 32 bits                    |     28 bits      |       |  bit  |   bit   |
+--------------------------------------------+------------------+-------+-------+---------+
</pre>

**Block Footer Format:**
<pre>
+--------------------------------------------+------------------+-------+-------+---------+
|              requested_size                |   block_size     |       | padded|  alloc  |
|                                            |                  |  00   |   1   |    1    |
|                 32 bits                    |     28 bits      |       |  bit  |   bit   |
+--------------------------------------------+------------------+-------+-------+---------+
</pre>

- The `block_size` field is 28 bits (but really 32 bits - see the note below).
It is the number of bytes for the entire block (header/footer, payload, and
padding)
- The `padded` bit is a boolean. It is 1 if the block was padded for alignment
and 0 if it was not.
- The `alloc` bit is a boolean. It is 1 if the block is allocated and 0 if it
is free.
- The `requested_size` field is 32 bits. It is the number of bytes that the
user requested.

> :thinking: The block size field is only 28 bits but technically it "uses"
> the lower four bits as part of the value. We can assume these bits are
> always 0 because the address must be divisible by 16 (alignment for the
> largest type in x86-64). This gives us an actual representation of 32 bits
> for the block size. For example, if the requested size is 13 bytes and the
> block size is 32 bytes, the header will be set to: 0x0000000000000023 and the
> footer will be set to: 0x0000000D00000023

> :thinking: Another example: let's say the requested size is 32 bytes. Adding
> 8 bytes for the header and 8 for the footer, we want to find a block of
> size 48 bytes. No padding is required because the requested payload size is
> a multiple of 16. Suppose we find a block of size 64. Splitting it would
> yield a 48-byte allocated block and a 16-byte free block. The remaining
> free block is a splinter, so we do not split. Instead, we return the entire
> 64-byte block to the user. The header is: 0x0000000000000043 and the footer
> is: 0x0000002000000043

# Getting Started

**Remember to use the `--strategy-option theirs` flag with the `git merge`
command as described in the `hw1` doc to avoid merge conflicts in the Gitlab
CI file.**

Fetch and merge the base code for `hw3` as described in `hw0` from the
following link: https://gitlab02.cs.stonybrook.edu/cse320/hw3

## Directory Structure

<pre>
repo/
├── .gitignore
├── .gitlab-ci.yml
└── hw3
    ├── include
    │   ├── debug.h
    │   └── sfmm.h
    ├── lib
    │   └── sfutil.o
    ├── Makefile
    ├── src
    │   ├── sfmm.c
    │   └── main.c
    └── tests
        └── sfmm_tests.c
</pre>

The base code includes the `build` folder in the `hw3` directory.

The `build` folder contains the object file for the `sfutil` library. This
library provides you with several functions to aid you with the implementation
of your allocator. <span style="color:red">**Do NOT delete this file as it
is an essential part of your homework assignment.**</span>

**Note:** `make clean` will not delete `sfutil.o` or the `lib` folder, but it
will delete all other contained `.o` files.

All functions for your allocator (`sf_malloc`, `sf_realloc`, and `sf_free`)
**must be** implemented in `src/sfmm.c`.

The program in `src/main.c` contains a basic example of using the initialization
and allocation functions together. Running `make` will create a `sfmm`
executable in the `bin` directory. This can be run using the command `bin/sfmm`.

The provided `Makefile` creates object files from the `.c` files in the `src`
directory, places the object files inside the `build` directory, and then links
the object files together to make executables, located in the `bin` directory.

> Any functions other than `sf_malloc`, `sf_free`, and `sf_realloc` **WILL NOT**
be graded.

> **DO NOT modify `sfmm.h` or the Makefile.** Both will be replaced when we run
> tests for grading. If you wish to add things to a header file, please create
> a new header file in the `include` folder

# Allocation Functions

You will implement the following three functions in the file `src/sfmm.c`.
The file `include/sfmm.h` contains the prototypes and documentation found here.

Standard C library functions set `errno` when there is an error. To avoid
conflicts with these functions, your allocation functions will set `sf_errno`, a
variable declared as `extern` in `sfmm.h`.

```c
/*
 * This is your implementation of sf_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, the pointer to a valid region of memory of the
 * requested size is returned, else NULL is returned and sf_errno as follows:
 *
 * If size is invalid (0 or greater than 4 pages), sf_errno is set to EINVAL
 * If the request cannot be satisfied, sf_errno is set to ENOMEM
 */
void *sf_malloc(size_t size);

/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, the pointer to a valid region of memory is
 * returned, else NULL is returned and sf_errno is set appropriately.
 *
 * If there is no memory available sf_realloc should set sf_errno to ENOMEM.
 * If sf_realloc is called with an invalid pointer sf_errno should be set to EINVAL.
 *
 * If sf_realloc is called with a valid pointer and a size of 0 it should free
 * the allocated block and return NULL.
 */
void* sf_realloc(void *ptr, size_t size);

/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function sf_malloc.
 *
 * If ptr is invalid, the function calls abort() to exit the program.
 */
void sf_free(void *ptr);
```

> :scream: <font color="red">Make sure these functions have these exact names
> and arguments. They must also appear in the correct file. If you do not name
> the functions correctly with the correct arguments, your program will not
> compile when we test it. **YOU WILL GET A ZERO**</font>

# Initialization Functions

In the `build` directory, we have provided you with the `sfutil.o` object file.
When linked with your program, this object file allows you to access the
`sfutil` library, which contains the following functions:

This library contains the following functions:

```c
/*
 * Any program using the sfmm library must call this function ONCE
 * before issuing any allocation requests. This function DOES NOT
 * allocate any space to your allocator.
 */
void sf_mem_init();

/*
 * Any program using the sfmm library must call this function ONCE
 * after all allocation requests are complete. If implemented cleanly,
 * your program should have no memory leaks in valgrind after this function
 * is called.
 */
void sf_mem_fini();

/*
 * This function changes the position of your program's break.
 * Calling this function increments the break by 1 page. On the first call,
 * the heap start and end variables are updated. On subsequent calls, only
 * the heap end variable is incremented by 1 page. Both of these can be accessed
 * through get_heap_start() and get_heap_end().
 *
 * @return On success, this function returns the previous program break.
 * If the break was increased, this value is a pointer to the start of the newly
 * allocated memory. On error, (void *) -1 is returned and sf_errno is set to ENOMEM.
 */
void *sf_sbrk();

/*
 * @return The starting address of the heap for your allocator.
 */
void *get_heap_start();

/*
 * @return The ending address of the heap for your allocator.
 */
void *get_heap_end();
```

The function `sf_mem_init` **MUST** be used to initialize memory. It is called
once by the user **BEFORE** using the allocator. Additionally, `sf_sbrk` is
called by `sf_malloc` on the first allocation and on subsequent allocations when
a block is not found (up to 4 calls).

> :scream: As these functions are provided in a pre-built .o file, the source
> is not available to you. You will not be able to debug these using gdb.
> You must treat them as black boxes.

# sf_sbrk

For this assignment, your implementation **MUST ONLY** use `sf_sbrk` to move the
heap breakpoint. DO NOT use any system calls such as **brk** or **sbrk** to
move the heap breakpoint.

`sf_sbrk` returns memory to your allocator in pages. Each page is 4096 bytes (4
KB) and you have access to 4 pages in total. This means your program can
allocate a total of 16,384 bytes (16 KB). Each call to `sf_sbrk` increments the
break pointer by 1 page and returns the previous break pointer.

The `sf_sbrk` function also keeps track of the starting and ending addresses of
the heap for you. You can get these addresses through the `get_heap_start` and
`get_heap_end` functions.

> :smile: A real allocator may use the **brk**/**sbrk** syscalls for small
> memory allocations and **mmap**/**munmap** syscalls for large allocations. To
> allow your program to use other functions provided by glibc, that rely on
> glibc's allocator, we prove a safe wrapper around **sbrk**. This makes it so
> that the heap breakpoint does not get altered by glibc's allocator and
> destroy the memory managed by your allocator.

# Implementation Details

## Memory Row Size

In this assignment we will assume that each "memory row" is 64-bits in size.

The table below lists the sizes of data types on x86-64 Linux Mint:

| C declaration | Data type | x86-64 Size (Bytes) |
| :--------------: | :----------------: | :----------------------: |
| char  | Byte | 1 |
| short | Word | 2 |
| int   | Double word | 4 |
| long int | Quadword | 8 |
| unsigned long | Quadword | 8 |
| pointer | Quadword | 8 |
| float | Single precision | 4 |
| double | Double precision | 8 |
| long double | Extended precision | 16

> :nerd: You can find these sizes yourself using the sizeof operator. For
example, `printf("%lu\n", sizeof(int))` prints 4

Consider this information when determining minimum block sizes and block
alignment.

## Free List Heads

In the file `include/sfmm.h`, you will see the following declarations:

```c
typedef struct {
    sf_free_header *head;
    uint16_t min;
    uint16_t max;
} free_list;

extern free_list seg_free_list[FREE_LIST_COUNT];
```

The `free_list` struct represents a single free list. It has a pointer to the
head of the list as well as the minimum and maximum sizes for blocks in the
list. Both sizes are **inclusive**.

The `extern` line declares an array of four `free_list` instances. There is one
instance for each free list. Each struct in the array is initialized in
`src/sfmm.c` with a `NULL` pointer for the free list head and the appropriate
min/max block sizes.

> :scream: You **MUST** use these pointers to access the free list heads.
> The helper functions discussed later, as well as the unit tests, use these
> variables to access your explicit free lists.

## Block Header & Footer Fields

The block header and footer formats and field sizes are specified in
`include/sfmm.h`

```c
#define HEADER_UNUSED_BITS 32
#define BLOCK_SIZE_BITS 28
#define TWO_ZEROES 2
#define PADDED_BITS 1
#define ALLOCATED_BITS 1
#define REQUESTED_SIZE_BITS 32


#define SF_HEADER_SIZE (HEADER_UNUSED_BITS + BLOCK_SIZE_BITS + TWO_ZEROES    \
                        + PADDED_BITS + ALLOCATED_BITS)

#define SF_FOOTER_SIZE SF_HEADER_SIZE

/*

                                      Format of a memory block
    +-----------------------------------------------------------------------------------------+
    |                                       64-bits wide                                      |
    +-----------------------------------------------------------------------------------------+

    +--------------------------------------------+------------------+-------+-------+---------+ <- header
    |                 unused                     |   block_size     |       | padded|  alloc  |
    |                                            |                  |  00   |   1   |    1    |
    |                 32 bits                    |     28 bits      |       |  bit  |   bit   |
    +--------------------------------------------+------------------+-------+-------+---------+
    |                                                                                         | Content of
    |                                   Payload and Padding                                   | the payload
    |                                     (N Memory Rows)                                     |
    |                                                                                         |
    |                                                                                         |
    +--------------------------------------------+------------------+-------+-------+---------+ <- footer
    |              requested_size                |   block_size     |       | padded|  alloc  |
    |                                            |                  |  00   |   1   |    1    |
    |                 32 bits                    |      28 bits     |       |  bit  |   bit   |
    +--------------------------------------------+------------------+-------+-------+---------+
*/
```

The file contains the general format for memory blocks in your allocator. The
block size is defined as

![alt text](img/block_size.png)

The block header is defined as:

```c
typedef struct {
    uint64_t      allocated : ALLOCATED_BITS;
    uint64_t         padded : PADDED_BITS;
    uint64_t     two_zeroes : TWO_ZEROES;
    uint64_t     block_size : BLOCK_SIZE_BITS;
    uint64_t         unused : HEADER_UNUSED_BITS;
} __attribute__((packed)) sf_header;
```

> :smile: The  `__attribute__((__packed__))` tells gcc to leave out all padding
> between members of the struct. In this way, the fields are forcibly placed
> next to each other.

To assist in accessing the next and previous pointers of a free block, an
additional struct `sf_free_header` is defined.

```c
struct sf_free_header{
    sf_header header;
    struct sf_free_header *next;
    struct sf_free_header *prev;
};

typedef struct sf_free_header sf_free_header;
```

Lastly, `include/sfmm.h` also provides a struct for accessing the footer.

```c
typedef struct {
    uint64_t      allocated : ALLOCATED_BITS;
    uint64_t         padded : PADDED_BITS;
    uint64_t     two_zeroes : TWO_ZEROES;
    uint64_t     block_size : BLOCK_SIZE_BITS;
    uint64_t requested_size : REQUESTED_SIZE_BITS;
} __attribute__((packed)) sf_footer;
```

> :thumbsup: You can cast blocks of memory with these structs in order to easily
> access the different fields of information of your memory allocator blocks.

**NOTE:** Assume all examples in the next sections use four free lists with the
block size constraints set in `sfmm.h`.

## Notes on sf_malloc

When implementing your `sf_malloc` function, first determine if the request size
is valid. The request size is invalid if it is 0 or it is greater than the
amount of memory your allocator has access to (4 pages). In the case of an
invalid request, set `sf_errno` to `EINVAL` and return `NULL`.

Once the request has been validated, you should add the sizes of the header
and footer, as well as any necessary padding to find the necessary block size.
Remember that you must start searching in the appropriate list, but you must
search the next lists in order if you cannot find a block. If a block is not
found in any list, you must use `sf_sbrk` to request more memory. If your
allocator cannot satisfy the request, your function must set `sf_errno` to
`ENOMEM` and return `NULL`.

### Notes on sf_sbrk

After each call to `sf_sbrk`, you must attempt to coalesce "backwards" (with the
block at the lower memory address in memory) in order to build blocks larger
than a page. After coalescing, add the entire new block to the **beginning** of
the correct list.

**Note:** Do not coalesce past the beginning of the heap start (given by the
`sf_heap_start` function)

# Notes on sf_free

When implementing `sf_free`, you must first verify that the pointer being
passed to your function belongs to an allocated block. This can be done by
examining the fields in the block header and footer. In this assignment, we
will consider the following cases for invalid pointers:

- The pointer is `NULL`
- The header of the block is before `heap_start` or block ends after
`heap_end`
- The `alloc` bit in the header or footer is 0
- The `requested_size`, `block_size`, and `padded` bits do not make sense when
put together. For example, if `requested_size + 16 != block_size`, you know
that the `padded` bit must be 1.
- The `padded` and `alloc` bits in the header and footer are inconsistent.

If an invalid pointer is passed to your function, you must call `abort` to exit
the program. Use the man page for the `abort` function to learn more about this.

After confirming that a valid pointer was given, you must free the block.
First, determine if the "next" block in memory can be coalesced. If it can,
remove the "next" block from its free list and combine the blocks. Then,
determine which list the newly freed block should be placed into. Insert it at
the head of this list. See the [Immediate Coalescing](#immediate-coalescing)
section for more details.

**Note that the only other times you should coalesce a block is after calling
`sf_sbrk` or in some cases of `sf_realloc`**

# Notes on sf_realloc

When implementing your `sf_realloc` function, you must first verify that the
pointer and size parameters passed to your function are valid. The criteria for
pointer validity are the same as those described in the `Notes on sf_free`
section above. If the size is 0, free the block and return `NULL`.

After verifying both parameters, consider the cases described below. Note that
in some cases, `sf_realloc` is more complicated than calling `sf_malloc` to
allocate more memory, `memcpy` to move the old memory to the new memory, and
`sf_free` to free the old memory.

## Reallocating to a Larger Size

When reallocating to a larger size, always follow these three steps:

1. Call `sf_malloc` to obtain a larger block

2. Call `memcpy` to copy the data in the block given by the user to the block
returned by `sf_malloc`

3. Call `sf_free` on the block given by the user (coalescing if necessary)

4. Return the block given to you by `sf_malloc` to the user

If `sf_malloc` returns `NULL`, `sf_realloc` must also return `NULL`. Note that
you do not need to set `sf_errno` in `sf_realloc` because `sf_malloc` should
take care of this.

## Reallocating to a Smaller Size

When reallocating to a smaller size, your allocator must use the block that was
returned by the user. You must attempt to split the returned block. There are
two cases for splitting:

- Splitting the returned block results in a splinter. In this case, do not
split the block. Leave the splinter in the block, update the header and footer
fields if necessary, and return the block back to the user. Note that you must
**not** attempt to coalesce the splinter.

Example:

<pre>
            b                                        b
+----------------------+                       +------------------------+
| allocated            |                       |   allocated.           |
| Blocksize: 64 bytes  |   sf_realloc(b, 32)   |   Block size: 64 bytes |
| payload: 48 bytes    |                       |   payload: 32 bytes    |
|                      |                       |                        |
|                      |                       |                        |
+----------------------+                       +------------------------+
</pre>

In the example above, splitting the block would have caused a 16-byte splinter.
Therefore, the block is not split. The `requested_size` field in the footer
is set to 32.

- The block can be split without creating a splinter. In this case, split the
block and update the block size fields in both headers. Free the remaining block
(i.e. coalesce if possible and insert the block into the head of the correct
free list). Return a pointer to the payload of the smaller block to the user.

Note that in both of these sub-cases, you return a pointer to the same block
that was given to you.

Example:

<pre>
            b                                        b
+----------------------+                       +------------------------+
| allocated            |                       | allocated |  free      |
| Blocksize: 64 bytes  |   sf_realloc(b, 16)   | 32 bytes  |  32 bytes. |
| payload: 48 bytes    |                       | payload:  |            |
|                      |                       | 16 bytes  | goes into  |
|                      |                       |           | free list 1|
+----------------------+                       +------------------------+
</pre>

# Helper Functions

The `sfutil` library additionally contains the following helper functions:

```c
/*
 * Function which outputs the state of the free-lists to stderr.
 * Performs checks on the placement of the header and footer
 * See sf_snapshot section for details on output format.
 */
void sf_snapshot();

/*
 * Function which prints human readable block format
 *
 * Note: If there are 'bad' addresses in your free list
 * this function will most likely segfault.
 *
 * @param block Address of the block header in memory.
 */
void sf_blockprint(void* block);

/*
 * Prints human readable block format from the address of the payload.
 * IE. subtracts header size from the data pointer to obtain the address
 * of the block header. Calls sf_blockprint internally to print.
 *
 * Note: If there are 'bad' addresses in your free list
 * this function will most likely segfault.
 *
 * @param data Pointer to payload data in memory
 * (value returned by sf_malloc).
 */
void sf_varprint(void *data);
```

We have provided these functions to help you visualize your free lists and
allocated blocks. We have also provided you with additional unit tests which
will check certain properties of each free block when a snapshot is being
performed and the snapshot verbose value is set to **true**.

## sf_blockprint and sf_varprint

These two functions print a visual representation of a memory block to `stdout`.
`sf_blockprint` takes the address of a memory block header and `sf_varprint`
takes the address of a memory block payload (i.e. the address that is returned
by `sf_malloc`).

The block size field displays the `block size << 4`. This is the total number
of bytes in the header, footer, payload, and padding if there is any. The value
printed is inclusive of the lower four bits.

## sf_snapshot

You can use this function to visualize your free lists. It starts by printing
out information about the snapshot: what kind of free lists are being used
(default is explicit), the size of the memory row in bytes, and the total
space taken from the heap so far. It then prints out each block in each list.

# Things to Remember

- Do not use prologue or epilogue blocks
- You are only working with 4 pages of memory (1 page = 4096 bytes). Allocations
over 4 pages should return NULL and set `sf_errno` to `EINVAL`.
- Make sure that memory returned to the user is aligned and padded correctly for
the system we use (64-bit Linux Mint).
- We will not grade using Valgrind. However, you are encouraged to use it to
detect alignment errors.

# Unit Testing

For this assignment, we will use Criterion to test your allocator. We have
provided a basic set of test cases and you will have to write your own as well.

You will use the Criterion framework alongside the provided helper functions to
ensure your allocator works exactly as specified.

In the `tests/sfmm_tests.c` file, there are nine unit test examples. These tests
check for the correctness of `sf_malloc`, `sf_realloc`, and `sf_free`. We
provide some basic assertions, but by no means are they extensive. It is your
job to ensure that your header/footer bits are set correctly and that blocks are
allocated/freed as specified.

## Compiling and Running Tests

When you compile your program with `make`, a `sfmm_tests` executable will be
created in the `bin` folder alongside the `main` executable. This can be run
with `bin/sfmm_tests`. To obtain more information about each test run, you can
use the verbose print option: `bin/sfmm_tests --verbose=0`.

# Writing Criterion Tests

The first test `Malloc_an_Integer` tests `sf_malloc`. It allocates space for an
integer and assigns a value to that space. It then runs an assertion to make
sure that the space returned by `sf_malloc` was properly assigned.

```c
cr_assert(*x == 4, "sf_malloc failed to give proper space for an int!");
```

The string after the assertion only gets printed to the screen if the assertion
failed (i.e. `*x != 4`). However, if there is a problem before the assertion,
such as a SEGFAULT, the unit test will print the error to the screen and
continue to run the rest of the unit tests.

For this assignment <font color="red">you must write 5 additional unit tests
which test new functionality and add them to `sfmm_tests.c` below the following
comment:</font>

```
//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################
```

> For additional information on Criterion library, take a look at the official
> documentation located [here](http://criterion.readthedocs.io/en/master/)! This
> documentation is VERY GOOD.

# Hand-in instructions
Make sure your directory tree looks like this and that your homework compiles.

<pre>
repo/
├── .gitignore
├── .gitlab-ci.yml
└── hw3
    ├── include
    │   ├── debug.h
    │   └── sfmm.h
    ├── lib
    │   └── sfutil.o
    ├── Makefile
    ├── src
    │   ├── sfmm.c
    │   └── main.c
    └── tests
        └── sfmm_tests.c
</pre>

This homework's tag is: `hw3`

<pre>
$ git submit hw3
</pre>

> :nerd: When writing your program try to comment as much as possible. Try to
> stay consistent with your formatting. It is much easier for your TA and the
> professor to help you if we can figure out what your code does quickly.
