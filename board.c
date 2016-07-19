#include "board.h"

#include <stdio.h>

uint8_t square(int file, int rank) {
    return file + 8 * rank;
}

void bb_print(uint64_t bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            if (BB_SQUARE(square(file, rank)) & bb) {
                printf("1");
            } else {
                printf(".");
            }

            if (file == 7) {
                printf("\n");
            } else {
                printf(" ");
            }
        }
    }
}

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

void board_reset(struct board *pos) {
    pos->white = BB_RANK_1 | BB_RANK_2;
    pos->black = BB_RANK_7 | BB_RANK_8;
    pos->kings = BB_E1 | BB_E8;
    pos->queens = BB_D1 | BB_D8;
    pos->rooks = BB_A1 | BB_A8 | BB_H1 | BB_H8;
    pos->bishops = BB_C1 | BB_C8 | BB_F1 | BB_F8;
    pos->knights = BB_B1 | BB_B8 | BB_G1 | BB_G8;
    pos->pawns = BB_RANK_2 | BB_RANK_7;

    pos->turn = true;

    pos->castling_rights = pos->rooks;

    pos->halfmove_clock = 0;
    pos->fullmove_number = 1;
}

char *board_board_fen(const struct board *pos, char *fen) {
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;

        for (int file = 0; file < 8; file++) {
            uint64_t bb = BB_SQUARE(square(file, rank));

            bool white = pos->white & bb;
            bool black = pos->black & bb;

            if (white || black) {
                if (empty) {
                    *fen++ = '0' + empty;
                    empty = 0;
                }

                if (pos->kings & bb) {
                    *fen++ = white ? 'K' : 'k';
                } else if (pos->queens & bb) {
                    *fen++ = white ? 'Q' : 'q';
                } else if (pos->rooks & bb) {
                    *fen++ = white ? 'R' : 'r';
                } else if (pos->bishops & bb) {
                    *fen++ = white ? 'B' : 'b';
                } else if (pos->knights & bb) {
                    *fen++ = white ? 'N' : 'n';
                } else if (pos->pawns & bb) {
                    *fen++ = white ? 'P' : 'p';
                }
            } else {
                empty++;
            }

            if (file == 7) {
                if (empty) {
                    *fen++ = '0' + empty;
                }

                if (rank > 0) {
                    *fen++ = '/';
                }
            }
        }
    }

    return fen;
}

char *board_fen(const struct board *pos, char *fen) {
    fen = board_board_fen(pos, fen);
    *fen++ = ' ';
    *fen++ = pos->turn ? 'w' : 'b';
    return fen;
}

bool board_set_fen(struct board *pos, const char *fen) {
    // pass
    return false;
}
