#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "board.h"

void test_bb_lsb() {
    puts("test_bb_lsb");

    assert(bb_lsb(3) == 0);
    assert(bb_lsb(8) == 3);
}

void test_bb_msb() {
    puts("test_bb_msb");

    assert(bb_msb(8) == 3);
    assert(bb_msb(BB_ALL) == 63);
}

void test_bb_popmsb() {
    puts("test_bb_popmsb");

    uint64_t bb = BB_ALL;
    uint8_t msb = bb_popmsb(&bb);
    assert(msb == 63);
    assert(bb == (BB_ALL ^ BB_H8));
}

void test_bb_popcount() {
    puts("test_bb_popcount");

    assert(bb_popcount(BB_A1 | BB_A2 | BB_H7) == 3);
    assert(bb_popcount(BB_VOID) == 0);
    assert(bb_popcount(BB_ALL) == 64);
}

void test_board_clear() {
    puts("test_board_clear");

    struct board pos;
    board_clear(&pos);
    assert((pos.white | pos.black) == 0);
    assert(pos.turn == true);
}

void test_board_reset() {
    puts("test_board_reset");

    struct board pos;
    board_reset(&pos);
    assert(BB_E1 & pos.kings);
    assert(BB_C7 & pos.black);
}

void test_board_shredder_fen() {
    puts("test_board_shredder_fen");

    struct board pos;
    board_reset(&pos);

    char fen[255];
    board_shredder_fen(&pos, fen);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w HAha - 0 1") == 0);
}

void test_board_set_fen() {
    puts("test_board_set_fen");

    struct board pos;
    assert(board_set_fen(&pos, "8/8/3k4/8/1q4N1/6K1/1p6/4R3 w - - 0 1"));
    assert((pos.white & pos.queens) == 0);
    board_print(&pos);
}

int main() {
    test_bb_lsb();
    test_bb_msb();
    test_bb_popmsb();
    test_bb_popcount();

    test_board_clear();
    test_board_reset();
    test_board_shredder_fen();
    test_board_set_fen();
}
