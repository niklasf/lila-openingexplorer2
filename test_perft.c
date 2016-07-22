#include "attacks.h"
#include "board.h"
#include "bitboard.h"
#include "move.h"

unsigned long perft(const board_t *pos, unsigned depth) {
    if (depth < 1) return 1;


    move_t moves[255];
    move_t *end = board_pseudo_legal_moves(pos, moves, BB_ALL, BB_ALL);

    unsigned long result = 0;

    for (move_t *current = moves; current < end; current++) {
        board_t pos_after = *pos;
        board_move(&pos_after, *current);
        if (!board_checkers(&pos_after, pos->turn)) result += perft(&pos_after, depth - 1);
    }

    return result;
}

void test_tricky() {
    puts("test_tricky");
    board_t pos;
    // Source: https://chessprogramming.wikispaces.com/Perft+Results

    // Position 2: Kiwipete by Peter McKenzie.
    puts("- Kiwipete");
    board_set_fen(&pos, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    assert(perft(&pos, 1) == 48);
    assert(perft(&pos, 2) == 2039);
    assert(perft(&pos, 3) == 97862);
    assert(perft(&pos, 4) == 4085603);

    // Position 3.
    puts("- 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    board_set_fen(&pos, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    assert(perft(&pos, 1) == 14);
    assert(perft(&pos, 2) == 191);
    assert(perft(&pos, 3) == 2812);
    assert(perft(&pos, 4) == 43238);
    assert(perft(&pos, 5) == 674624);
}

int main() {
    attacks_init();

    puts("starting pos");
    board_t pos;
    board_reset(&pos);
    for (unsigned depth = 0; depth < 5; depth++) {
        printf("%d: %d\n", depth, perft(&pos, depth));
    }

    test_tricky();
    return 0;
}
