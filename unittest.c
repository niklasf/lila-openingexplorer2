#include <assert.h>
#include <stdio.h>

#include "board.h"

void test_bb_popcount() {
    puts("test_bb_popcount");

    assert(bb_popcount(BB_A1 | BB_A2 | BB_H7) == 3);
    assert(bb_popcount(BB_VOID) == 0);
    assert(bb_popcount(BB_ALL) == 64);
}

void test_board_clear() {
    puts("test_clear");

    struct board pos;
    board_clear(&pos);
    assert((pos.white | pos.black) == 0);
    assert(pos.turn == true);
}

void test_board_reset() {
    puts("test_reset");

    struct board pos;
    board_reset(&pos);
    assert(BB_E1 & pos.kings);
    assert(BB_C7 & pos.black);

    bb_print(pos.pawns);
}

void test_board_fen() {
    puts("test_fen");

    struct board pos;
    board_reset(&pos);

    char fen[255];
    board_fen(&pos, fen);
    puts(fen);
}

void test_board_set_fen() {
    puts("test_set_fen");

    struct board pos;
    assert(board_set_fen(&pos, "8/8/3k4/8/1q4N1/6K1/1p6/4R3 w - - 0 1"));
    assert((pos.white & pos.queens) == 0);
}

int main() {
    test_bb_popcount();

    test_board_clear();
    test_board_reset();
    test_board_fen();
    test_board_set_fen();
}
