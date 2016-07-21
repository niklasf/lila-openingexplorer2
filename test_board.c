#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "bitboard.h"

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
    char fen[255];

    board_reset(&pos);
    board_shredder_fen(&pos, fen);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w HAha - 0 1") == 0);

    board_clear(&pos);
    pos.white |= BB_D1;
    pos.black |= BB_D8;
    pos.queens |= BB_D1 | BB_D8;
    board_shredder_fen(&pos, fen);
    assert(strcmp(fen, "3q4/8/8/8/8/8/8/3Q4 w - - 0 1") == 0);
}

void test_board_set_fen() {
    puts("test_board_set_fen");

    struct board pos;
    assert(board_set_fen(&pos, "8/8/3k4/8/1q4N1/6K1/1p6/4R3 w - - 2 15"));
    assert((pos.white & pos.queens) == 0);
    assert(pos.turn == true);
    board_print(&pos);

    char fen[255];
    board_shredder_fen(&pos, fen);
    puts(fen);

    assert(board_set_fen(&pos, "rnbqk1nr/ppppppbp/6p1/8/8/5NP1/PPPPPPBP/RNBQK2R b KQkq - 2 3"));
    board_shredder_fen(&pos, fen);
    puts(fen);
    assert(pos.castling == (BB_A1 | BB_A8 | BB_H1 | BB_H8));
    assert(pos.queens & pos.white & BB_D1);
    assert(pos.queens & pos.black & BB_D8);
}

void test_board_is_insufficient_material() {
    puts("test_board_is_insufficient_material");

    struct board pos;

    // King vs king + 2 bishops of the same color.
    assert(board_set_fen(&pos, "k1K1B1B1/8/8/8/8/8/8/8 w - - 7 32"));
    assert(board_is_insufficient_material(&pos));

    // Add bishop of opposite color for the weaker side.
    pos.bishops |= BB_B8;
    pos.black |= BB_B8;
    assert(!board_is_insufficient_material(&pos));
}

int main() {
    test_board_clear();
    test_board_reset();
    test_board_shredder_fen();
    test_board_set_fen();
    test_board_is_insufficient_material();
}
