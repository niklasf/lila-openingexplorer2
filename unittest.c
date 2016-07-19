#include <assert.h>
#include <stdio.h>

#include "board.h"

void test_clear() {
    puts("test_clear");

    struct board pos;
    board_clear(&pos);
    assert((pos.white | pos.black) == 0);
    assert(pos.turn == true);
}

void test_reset() {
    puts("test_reset");

    struct board pos;
    board_reset(&pos);
    assert(BB_E1 & pos.kings);
    assert(BB_C7 & pos.black);
}

void test_set_fen() {
    puts("test_set_fen");

    struct board pos;
    assert(board_set_fen(&pos, "8/8/3k4/8/1q4N1/6K1/1p6/4R3 w - - 0 1"));
    assert((pos.white & pos.queens) == 0);
}

int main() {
    test_clear();
    test_reset();
    test_set_fen();
}
