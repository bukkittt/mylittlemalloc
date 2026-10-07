CC = gcc
CFLAGS = -Wall -Werror -g

# Default target
all: memgrind memtest

# Compile memgrind executable
memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o memgrind memgrind.o mymalloc.o

# compile memtest exec
memtest: memtest.o mymalloc.o
	$(CC) $(CFLAGS) -o memtest memtest.o mymalloc.o

# Compile object files
memgrind.o: memgrind.c mymalloc.h
	$(CC) $(CFLAGS) -c memgrind.c

memtest.o: memtest.c mymalloc.h
	$(CC) $(CFLAGS) -c memtest.c

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c

# Clean up build artifacts
clean:
	rm -f *.o memgrind memtest
