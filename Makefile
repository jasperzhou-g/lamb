CC = gcc
CFLAGS = -g -Wall -Wpedantic
objects = main.o lexer.o error.o parser.o

all: $(objects)
	$(CC) $(objects) -o lamb

$(objects): %.o: ./src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf *.c *.o lamb