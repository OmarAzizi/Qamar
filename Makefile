CC = gcc
CFLAGS = -g -Wall 
SOURCE = main.c chunk.c memory.c debug.c value.c vm.c compiler.c scanner.c object.c table.c
OBJECTS = $(SOURCE:.c=.o)
LIBS = -ledit

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o onyx $(OBJECTS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

clean:
	rm -rf *.o *~

clean-all:
	rm -rf *.o *~ onyx
