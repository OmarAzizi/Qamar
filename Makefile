CC = gcc
CFLAGS = -Wall 
SOURCE = main.c chunk.c memory.c debug.c value.c vm.c compiler.c scanner.c object.c table.c
OBJECTS = $(SOURCE:.c=.o)

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o onyx $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o *~

clean-all:
	rm -rf *.o *~ onyx
