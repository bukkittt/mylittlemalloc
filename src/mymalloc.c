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

// ^ is xtra storage and because its not static, client code can see it.

static int initialized = 0; // flag to indicate if the heap has been initialized

static void leak_detection() {
    int leaked_object = 0;
    int leaked_bytes = 0;
    int offset = 0;

    while(offset < MEMLENGTH){
        struct header *current_header = (struct header*)((char *)first_chunk + offset);
        if(current_header -> size<=0 || current_header-> size> MEMLENGTH) break;

        if(current_header -> allocated){
            leaked_object ++;
            leaked_bytes += current_header-> size;
        }

        offset = offset + HEADERSIZE + current_header->size;
    }

    if (leaked_object > 0){
        fprintf(stderr, "mymalloc: %d bytes leaked in %d objects.\n", leaked_bytes, leaked_object);
    }
}
    

static void initialize_heap() {
    atexit(leak_detection); // register the leak detection function to be called at program exit
    
    struct header *first_chunk = (struct header *)heap.bytes; // pointer to the first chunk in the heap
    first_chunk->size = MEMLENGTH-HEADERSIZE; // set the size of the first chunk to the total heap size
    first_chunk->allocated = 0; // mark the first chunk as free
    initialized = 1; // set the initialized flag to true
}


void * mymalloc (size_t size, char *file, int line){
    if(!initialized) initialize_heap();
    if (size == 0) return NULL;
    
    //fixed so we reject impossible sizes before rounding
    if(size > MEMLENGTH - HEADERSIZE) { 
	    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
	    return NULL;
    }

    int alsize = (int)((size + 7) & ~(size_t)7);// size should be multiple of 8

    int offset = 0;
    while(offset < MEMLENGTH){
        struct header *current_header = (struct header*)((char*)first_chunk + offset);
        if(! current_header -> allocated && current_header-> size >= alsize){
            //chunk is free and can fit alsize
            if(current_header-> size >= alsize + HEADERSIZE + 8){
                //chunk is free and there is enough space to put alsize + header + at least 8 bytes of extra space
                int temp_size = current_header-> size;
                current_header-> size = alsize;

                struct header *next_header= (struct header *) ((char *)current_header + HEADERSIZE + alsize);
                next_header-> size = temp_size - alsize - HEADERSIZE;
                next_header-> allocated = 0;
            }
            current_header->allocated=1;
            return (void *)((char *) current_header + HEADERSIZE);
        }

        offset = offset + HEADERSIZE + current_header->size;
    }

    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree (void *ptr, char *file, int line){
    if (!initialized) initialize_heap();
    if (ptr == NULL) return;
    //err1: address not obtained from malloc
    if ((char *)ptr < heap.bytes + HEADERSIZE || (char *)ptr >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    int offset = 0;
    int found = 0;//if the point going to free exist
    struct header *target = NULL;//the pointer going to free

    while (offset < MEMLENGTH) {
       struct header *current = (struct header *)(heap.bytes + offset);
        void *expectedmallocptr = (void *)((char *)current + HEADERSIZE);
        
        if (ptr == expectedmallocptr) {
            found = 1;
            target = current;
            break;
        }
        offset += HEADERSIZE + current->size;
    }

    // err2: Address not at the start of a chunk
    if (!found) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    // err3: double free
    if (!target->allocated) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    target->allocated = 0;

    // Coalesce adjacent free blocks
    offset = 0;
    while (offset < MEMLENGTH) {
        struct header *current_header = (struct header*)(heap.bytes + offset);
        int next_offset = offset + HEADERSIZE + current_header->size;
        
        if (next_offset < MEMLENGTH) {
            struct header *next = (struct header *)(heap.bytes + next_offset);
            if (!current_header->allocated && !next->allocated) {
                // Merge next block into current block
                current_header->size += HEADERSIZE + next->size;
                continue;
            }
        }
        offset = offset + HEADERSIZE + current_header->size;
    }
}
