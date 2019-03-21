.PHONY: clean
objects = main.o buffer.o csapp.o
CFLAGS =  -std=gnu11 -Wall -lpthread -ggdb3 -O0 

buffer: $(objects) buffer.c
	gcc $(CFLAGS) -o buffer $(objects)

clean:
	rm -f buffer light *.o

