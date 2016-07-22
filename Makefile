CC=clang
CFLAGS=-Wall -mpopcnt -mbmi2 -std=gnu99
LDFLAGS=

OBJS = square.o bitboard.o board.o test_perft.o test_bitboard.o test_attacks.o test_board.o

.PHONY: test
test: .depend test_bitboard test_attacks test_board test_perft
	./test_bitboard
	./test_attacks
	./test_board

test_bitboard: test_bitboard.o bitboard.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_attacks: test_attacks.o attacks.o bitboard.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_board: test_board.o board.o attacks.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_perft: test_perft.o board.o attacks.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

.depend:
	$(CC) $(DEPENDFLAGS) -MM $(OBJS:.o=.c) > $@ 2> /dev/null

-include .depend
