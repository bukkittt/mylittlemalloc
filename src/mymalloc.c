#include "mymalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMLENGTH 4096
#define HEADERSIZE 8

static union {
    char bytes[MEMLENGTH];
    double not_used;// ensures that the union is aligned to 8 bytes
} heap;

struct header {
    int size; // size of the chunk (NOT including the header)
    int allocated; // 1 if allocated, 0 if free
    //whether the chunk is allocated or free. Given the location of one chunk, you can simply add its size to the location to get the next chunk.
    //Note the pointer returned by mymalloc() must point to the payload, not the chunk header.
};

struct header *first_chunk = (struct header *)heap.bytes; // pointer to the first chunk in the heap

static int initialized = 0; // flag to indicate if the heap has been initialized

static void leak_detection() {
   //2.2 to be implemented in mymalloc.c
}

static void initialize_heap() {
    atexit(leak_detection); // register the leak detection function to be called at program exit


    first_chunk->size = MEMLENGTH-HEADERSIZE; // set the size of the first chunk to the total heap size
    first_chunk->allocated = 0; // mark the first chunk as free
    initialized = 1; // set the initialized flag to true
}


void * mymalloc (size_t size, char *file, int line);
void   myfree (void *ptr, char *file, int line);
