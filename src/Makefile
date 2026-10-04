CC = gcc
CFLAGS = -Wall -Werror -g

# Default target
all: memgrind

# Compile memgrind executable
memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o memgrind memgrind.o mymalloc.o

# Compile object files
memgrind.o: memgrind.c mymalloc.h
	$(CC) $(CFLAGS) -c memgrind.c

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c

# Clean up build artifacts
clean:
	rm -f *.o memgrind
