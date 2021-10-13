CC = clang
CFLAGS = -Wall -Werror -Wpedantic -Wextra
LFLAGS = -lm

all: encode decode entropy

encode: stack.o encode.o code.o code.o huffman.o io.o node.o pq.o
	$(CC) -o encode stack.o code.o encode.o huffman.o io.o node.o pq.o $(LFLAGS)

decode: decode.o stack.o huffman.o io.o node.o code.o pq.o
	$(CC) -o decode decode.o stack.o huffman.o io.o node.o code.o pq.o $(LFLAGS)

entropy:
	$(CC) $(CFLAGS) -o entropy entropy.c $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f encode decode entropy *.o

format:
	clang-format -i -style=file *.c *.h
