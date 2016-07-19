CC=clang
CFLAGS=-Wall
LDFLAGS=

.PHONY: test
test: unittest
	./unittest

unittest: unittest.o board.o
	$(CC) -o unittest unittest.o board.o $(LDFLAGS)

test.o: test.c board.h
	$(CC) $(CFLAGS) -g -O -c test.c

board.o: board.c board.h
	$(CC) $(CFLAGS) -g -O -c board.c -o board.o
