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
        if (!board_checkers(&pos_after)) result += perft(&pos_after, depth - 1);
    }

    return result;
}

int main() {
    attacks_init();

    board_t pos;
    board_reset(&pos);
    for (unsigned depth = 0; depth < 5; depth++) {
        printf("%d: %d\n", depth, perft(&pos, depth));
    }

    return 0;
}
