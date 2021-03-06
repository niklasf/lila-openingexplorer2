CC = clang
CFLAGS = -Wall -Werror -mpopcnt -mbmi2 -std=gnu99 -fPIE -fstack-protector-all -O3
LDFLAGS = -Wl,-z,now -Wl,-z,relro -levent -lkyotocabinet

OBJS = encode.o square.o bitboard.o board.o pgn.o \
       test_encode.o test_perft.o test_bitboard.o test_attacks.o test_board.o \
       test_pgn.o

all: explorer index_master test_bitboard test_attacks test_board test_perft test_encode test_pgn

explorer: main.o encode.o pgn.o attacks.o board.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

index_master: index_master.o encode.o pgn.o attacks.o board.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: test
test: .depend test_bitboard test_attacks test_board test_perft test_encode test_pgn
	./test_bitboard
	./test_attacks
	./test_board
	./test_encode
	./test_pgn

test_bitboard: test_bitboard.o bitboard.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_attacks: test_attacks.o attacks.o bitboard.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_board: test_board.o board.o attacks.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_perft: test_perft.o board.o attacks.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_encode: test_encode.o encode.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_pgn: test_pgn.o pgn.o board.o attacks.o bitboard.o move.o square.o
	$(CC) -o $@ $^ $(LDFLAGS)

.depend:
	$(CC) $(DEPENDFLAGS) -MM $(OBJS:.o=.c) > $@ 2> /dev/null

-include .depend
