#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "bitboard.h"
#include "square.h"
#include "attacks.h"

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

void test_board_attacks_to() {
    puts("test_board_attacks_to");

    struct board pos;
    board_reset(&pos);
    uint64_t attacks = board_attacks_to(&pos, SQ_F6);
    assert(attacks == (BB_G8 | BB_E7 | BB_G7));

    assert(board_set_fen(&pos, "r1bqk2r/pppp1Bpp/2n2n2/2b1p1N1/4P3/8/PPPP1PPP/RNBQK2R b KQkq - 0 5"));
    attacks = board_attacks_to(&pos, SQ_E8);
    assert(attacks & BB_F7);
}

void test_board_attacks_from() {
    puts("test_board_attacks_from");

    uint64_t attacks;
    struct board pos;
    board_reset(&pos);

    attacks = board_attacks_from(&pos, SQ_B1);
    assert(attacks == (BB_A3 | BB_C3 | BB_D2));

    attacks = board_attacks_from(&pos, SQ_D1);
    assert(attacks == (BB_C1 | BB_C2 | BB_D2 | BB_E2 | BB_E1));

    assert(board_set_fen(&pos, "r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4"));
    attacks = board_attacks_from(&pos, SQ_H1);
    bb_print(attacks);
    assert(attacks == (BB_E1 | BB_F1 | BB_G1 | BB_H2));
}

void test_board_checkers() {
    puts("test_board_checkers");

    struct board pos;
    board_reset(&pos);
    assert(!board_checkers(&pos));

    assert(board_set_fen(&pos, "r1bqk2r/pppp1Bpp/2n2n2/2b1p1N1/4P3/8/PPPP1PPP/RNBQK2R b KQkq - 0 5"));
    assert(pos.turn == false);
    uint64_t checkers = board_checkers(&pos);
    assert(checkers == BB_F7);
}

void test_board_pseudo_legal_moves() {
    puts("test_board_pseudo_legal_moves");

    struct board pos;
    assert(board_set_fen(&pos, "rnbqk2r/ppppppbp/5np1/8/3P4/5NP1/PPP1PPBP/RNBQK2R b KQkq d3 0 4"));

    move_t moves[255];
    move_t *end = board_pseudo_legal_moves(&pos, moves, BB_ALL, BB_ALL);
    for (move_t *current = moves; current < end; current++) {
        char uci[6];
        move_uci(*current, uci);
        puts(uci);
    }
}

void test_board_pseudo_legal_ep() {
    puts("test_board_pseudo_legal_ep");

    struct board pos;
    assert(board_set_fen(&pos, "4k3/8/8/8/4Pp2/8/8/4K3 b - e3 0 1"));
    move_t ep = move_make(SQ_F4, SQ_E3, 0);

    move_t moves[255];
    move_t *end = board_pseudo_legal_moves(&pos, moves, BB_ALL, BB_ALL);

    bool found = false;
    for (move_t *current = moves; current < end; current++) {
        if (*current == ep) found = true;

        char uci[6];
        move_uci(*current, uci);
        puts(uci);
    }

    assert(found);
}

void test_board_legal_moves() {
    puts("test_board_legal_moves");

    board_t pos;
    assert(board_set_fen(&pos, "r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4"));
    board_print(&pos);

    move_t moves[255];
    move_t *end = board_legal_moves(&pos, moves, BB_ALL, BB_ALL);
    assert((end - moves) == 32);
}

int main() {
    attacks_init();

    test_board_clear();
    test_board_reset();
    test_board_shredder_fen();
    test_board_set_fen();
    test_board_is_insufficient_material();
    test_board_attacks_to();
    test_board_attacks_from();
    test_board_checkers();
    test_board_pseudo_legal_moves();
    test_board_pseudo_legal_ep();
    test_board_legal_moves();
    return 0;
}
