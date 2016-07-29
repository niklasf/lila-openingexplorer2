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
    assert((pos.occupied_co[0] | pos.occupied_co[1]) == 0);
    assert(pos.turn == true);
}

void test_board_reset() {
    puts("test_board_reset");

    struct board pos;
    board_reset(&pos);
    assert(BB_E1 & pos.occupied[kKing]);
    assert(BB_C7 & pos.occupied[kPawn]);
}

void test_board_shredder_fen() {
    puts("test_board_shredder_fen");

    struct board pos;
    char fen[255];

    board_reset(&pos);
    board_shredder_fen(&pos, fen);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w HAha - 0 1") == 0);

    board_clear(&pos);
    board_set_piece_at(&pos, SQ_D1, kQueen, kWhite);
    board_set_piece_at(&pos, SQ_D8, kQueen, kBlack);
    board_shredder_fen(&pos, fen);
    assert(strcmp(fen, "3q4/8/8/8/8/8/8/3Q4 w - - 0 1") == 0);
}

void test_board_set_fen() {
    puts("test_board_set_fen");

    struct board pos;
    assert(board_set_fen(&pos, "8/8/3k4/8/1q4N1/6K1/1p6/4R3 w - - 2 15"));
    assert(board_pieces(&pos, kQueen, kWhite) == 0);
    assert(pos.turn == true);

    char fen[255];
    board_shredder_fen(&pos, fen);

    assert(board_set_fen(&pos, "rnbqk1nr/ppppppbp/6p1/8/8/5NP1/PPPPPPBP/RNBQK2R b KQkq - 2 3"));
    board_shredder_fen(&pos, fen);
    assert(pos.castling == (BB_A1 | BB_A8 | BB_H1 | BB_H8));
    assert(board_pieces(&pos, kQueen, kWhite) & BB_D1);
    assert(board_pieces(&pos, kQueen, kBlack) & BB_D8);
}

void test_board_is_insufficient_material() {
    puts("test_board_is_insufficient_material");

    struct board pos;

    // King vs king + 2 bishops of the same color.
    assert(board_set_fen(&pos, "k1K1B1B1/8/8/8/8/8/8/8 w - - 7 32"));
    assert(board_is_insufficient_material(&pos));

    // Add bishop of opposite color for the weaker side.
    board_set_piece_at(&pos, SQ_B8, kBishop, kBlack);
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
    assert(!board_checkers(&pos, pos.turn));

    assert(board_set_fen(&pos, "r1bqk2r/pppp1Bpp/2n2n2/2b1p1N1/4P3/8/PPPP1PPP/RNBQK2R b KQkq - 0 5"));
    assert(pos.turn == false);
    uint64_t checkers = board_checkers(&pos, pos.turn);
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

    move_t moves[255];
    move_t *end = board_legal_moves(&pos, moves, BB_ALL, BB_ALL);
    assert((end - moves) == 32);
}

void test_legal_promotion() {
    puts("test_legal_promotion");

    board_t pos;
    assert(board_set_fen(&pos, "4k3/1P6/8/8/8/8/8/4K3 w - - 0 1"));
    move_t moves[255];
    move_t *end = board_legal_moves(&pos, moves, BB_ALL, BB_ALL);

    move_t needle = move_make(SQ_B7, SQ_B8, kQueen);
    bool found = false;
    bool found_uci = false;
    for (move_t *current = moves; current < end; current++) {
        if (*current == needle) found = true;

        char uci[6];
        move_uci(*current, uci);
        if (strcmp(uci, "b7b8q") == 0) found_uci = true;

        puts(uci);
    }

    assert(found);
    assert(found_uci);
}

void test_board_zobrist_hash() {
    puts("test_board_zobrist_hash");
    board_t pos;

    board_reset(&pos);
    assert(board_zobrist_hash(&pos, POLYGLOT) == 0x463b96181691fc9cULL);

    assert(board_set_fen(&pos, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"));
    assert(board_zobrist_hash(&pos, POLYGLOT) == 0x823c9b50fd114196ULL);

    assert(board_set_fen(&pos, "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 1 3"));
    assert(board_zobrist_hash(&pos, POLYGLOT) == 0x652a607ca3f242c1ULL);
}

void test_board_parse_san() {
    puts("test_board_parse_san");
    board_t pos;
    move_t move;

    board_reset(&pos);
    assert(board_parse_san(&pos, "e4", &move));
    assert(move == move_make(SQ_E2, SQ_E4, 0));

    assert(board_set_fen(&pos, "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R b KQkq - 5 4"));
    assert(board_parse_san(&pos, "Nxe4", &move));
    assert(move == move_make(SQ_F6, SQ_E4, 0));
}

void test_board_san() {
    puts("test_board_san");
    board_t pos;
    char san[LEN_SAN];

    // Double pawn move.
    board_reset(&pos);
    board_san(&pos, move_make(SQ_H2, SQ_H4, kNone), san);
    assert(strcmp(san, "h4") == 0);

    // Promotion.
    assert(board_set_fen(&pos, "4k3/8/8/8/8/8/6p1/4K3 b - - 0 1"));
    board_san(&pos, move_make(SQ_G2, SQ_G1, kRook), san);
    assert(strcmp(san, "g1=R+") == 0);

    // Not ambiguous because of pin.
    assert(board_set_fen(&pos, "4k3/8/8/b7/8/2N3N1/8/4K3 w - - 0 1"));
    board_san(&pos, move_make(SQ_G3, SQ_E4, 0), san);
    puts(san);
    assert(strcmp(san, "Ne4") == 0);

    // Ambiguous.
    board_remove_piece_at(&pos, SQ_A5);
    board_san(&pos, move_make(SQ_G3, SQ_E4, 0), san);
    assert(strcmp(san, "Nge4") == 0);
}

void test_board_evasive_capture() {
    puts("test_board_evasive_capture");
    board_t pos;
    assert(board_set_fen(&pos, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P1RPP/R2Q2K1 b af - 1 1"));
    board_move(&pos, move_make(SQ_B6, SQ_F2, 0));

    move_t moves[255];
    move_t *end = board_legal_moves(&pos, moves, BB_ALL, BB_ALL);
    assert(end - moves == 3);
}

void test_board_pin() {
    puts("test_board_pin");

    board_t pos;
    assert(board_set_fen(&pos, "4k3/8/8/b7/8/2N3N1/8/4K3 w - - 0 1"));

    move_t moves[255];
    move_t *end = board_legal_moves(&pos, moves, BB_ALL, BB_ALL);
    bool found_Nge4 = false, found_Nce4 = false;
    for (move_t *current = moves; current < end; current++) {
        char san[LEN_SAN], uci[LEN_UCI];
        move_uci(*current, uci);
        board_san(&pos, *current, san);
        printf("- %s %s\n", uci, san);

        if (*current == move_make(SQ_C3, SQ_E4, kNone)) found_Nce4 = true;
        if (*current == move_make(SQ_G3, SQ_E4, kNone)) found_Nge4 = true;
    }

    assert(found_Nge4);
    assert(!found_Nce4);
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
    test_legal_promotion();
    test_board_zobrist_hash();
    test_board_parse_san();
    test_board_san();
    test_board_evasive_capture();
    test_board_pin();
    return 0;
}
