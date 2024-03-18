CC = gcc
CFLAGS = -Wall -Wextra
SOURCE = main.c chunk.c memory.c debug.c value.c
OBJECTS = $(SOURCE:.c=.o)

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o clox $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o *~ clox
