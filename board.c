#include "board.h"

void board_clear(struct board *pos) {
    pos->white = 0;
    pos->black = 0;
    pos->kings = 0;
    pos->queens = 0;
    pos->rooks = 0;
    pos->bishops = 0;
    pos->knights = 0;
    pos->pawns = 0;

    pos->turn = true;

    pos->castling_rights = 0;

    pos->halfmove_clock = 0;
    pos->fullmove_number = 1;
}

bool board_set_fen(struct board *pos, const char *fen) {
    // pass
    return false;
}
