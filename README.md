# mylittlemalloc
Assignment 1: My little Malloc()

Mark Filip : mf1185

Tinson Dong: td584

# Test Plan:

correctness.c:

N    Scenario                        expected output                      exit
1    free(&x), local variable        free: Inappropriate pointer(...)     2
2    free(p+1)                       free: Inappropriate pointer(...)     2
3    double free                     free: Inappropriate pointer(...)     2
4    free(&global_var)               free: Inappropriate pointer(...)     2
5    leak n objects              mymalloc: 128 bytes leaked in n objects  0
6    everything freed                N/A                                  0

1. malloc() returns memory that does not overlap other objects.
   If objects overlapped, writing to one would change another.
   Test: memtest fills the heap with 64 objects, writes a different byte
   value into each, then checks every byte. It prints "0 incorrect bytes"
   on success.

2. free() deallocates memory.
   If it did not, memory would never become available again.
   Test: correctness fills the heap with one object, checks that
   malloc(1) now fails, frees the object, and checks that the same large
   request succeeds again. It also checks that a freed object's address is
   handed out by the next malloc of the same size.

3. Adjacent free chunks are coalesced.
   If not, a large request would fail even after many small neighbouring
   chunks were freed.
   Test: correctness fills the heap with 256 small chunks, frees them
   (first to last, then last to first), and requests the largest possible
   object. It also frees a chunk between two free neighbours and checks all
   three merge.

4. Bad calls to free() are detected.
   Each must print "free: Inappropriate pointer (file:line)" and exit with
   status 2.
   Test: ./correctness 1-4 free a local variable, a pointer into the
   middle of an object, the same pointer twice, and a global variable.

5. Leaks are detected and reported correctly.
   Test: correctness 5 leaks 3 objects and must print
   "mymalloc: 128 bytes leaked in 3 objects."; correctness 6 frees
   everything and must print nothing.

6. The library handles many operations without breaking.
   Test: memgrind runs the 5-task workload 50 times. Any failed malloc
   prints an error, and memgrind prints a warning at the end, so a clean
   run means no failures.
