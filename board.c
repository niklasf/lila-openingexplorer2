#include "board.h"

#include <stdio.h>

char *square_name(uint8_t square, char *name) {
    *name++ = 'a' + square_file(square);
    *name++ = '1' + square_rank(square);
    return name;
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

char board_piece_at(const struct board *pos, uint8_t square) {
    uint64_t bb = BB_SQUARE(square);

    bool white = pos->white & bb;
    bool black = pos->black & bb;

    if (white || black) {
        if (pos->kings & bb) {
            return white ? 'K' : 'k';
        } else if (pos->queens & bb) {
            return white ? 'Q' : 'q';
        } else if (pos->rooks & bb) {
            return white ? 'R' : 'r';
        } else if (pos->bishops & bb) {
            return white ? 'B' : 'b';
        } else if (pos->knights & bb) {
            return white ? 'N' : 'n';
        } else if (pos->pawns & bb) {
            return white ? 'P' : 'p';
        }
    }

    return 0;
}

void board_print(const struct board *pos) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            char piece = board_piece_at(pos, square(file, rank));
            printf("%c", piece ? piece : '.');
            printf("%c", file == 7 ? '\n' : ' ');
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

    pos->ep_square = 0;
    pos->castling_rights = pos->rooks;

    pos->halfmove_clock = 0;
    pos->fullmove_number = 1;
}

char *board_board_fen(const struct board *pos, char *fen) {
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;

        for (int file = 0; file < 8; file++) {
            char piece = board_piece_at(pos, square(file, rank));

            if (piece) {
                if (empty) {
                    *fen++ = '0' + empty;
                    empty = 0;
                }

                *fen++ = piece;
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

char *board_castling_shredder_fen(const struct board *pos, char *castling_fen) {
    uint64_t white_castling = pos->castling_rights & BB_RANK_1;
    uint64_t black_castling = pos->castling_rights & BB_RANK_8;

    if (!white_castling && !black_castling) {
        *castling_fen++ = '-';
        return castling_fen;
    }

    while (white_castling) {
        *castling_fen++ = 'A' + square_file(bb_popmsb(&white_castling));
    }

    while (black_castling) {
        *castling_fen++ = 'a' + square_file(bb_popmsb(&black_castling));
    }

    return castling_fen;
}

char *board_shredder_fen(const struct board *pos, char *fen) {
    fen = board_board_fen(pos, fen);
    *fen++ = ' ';

    *fen++ = pos->turn ? 'w' : 'b';
    *fen++ = ' ';

    fen = board_castling_shredder_fen(pos, fen);
    *fen++ = ' ';

    if (pos->ep_square) fen = square_name(pos->ep_square, fen);
    else *fen++ = '-';

    *fen++ = ' ';
    int hmvc = (pos->halfmove_clock > 9999) ? 9999 : pos->halfmove_clock;
    fen += sprintf(fen, "%d", hmvc);

    *fen++ = ' ';
    int fmvn = (pos->fullmove_number > 9999) ? 9999 : pos->fullmove_number;
    fen += sprintf(fen, "%d", fmvn);

    return fen;
}

bool board_set_fen(struct board *pos, const char *fen) {
    uint64_t black = 0, white = 0;
    uint64_t kings, queens, rooks, bishops, knights, pawns;
    kings = queens = rooks = bishops = knights = pawns = 0;
    bool turn;

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file <= 7; file++) {
            uint64_t bb = BB_SQUARE(square(file, rank));
            char c = *fen++;
            if (!c) return false;

            if (c >= 'a' && c <= 'z') black |= bb;
            if (c >= 'A' && c <= 'Z') white |= bb;

            if (c >= '1' && c <= '8') {
                file += c - '1';
                continue;
            }

            switch (c) {
                case 'k':
                case 'K':
                    kings |= bb;
                    continue;
                case 'q':
                case 'Q':
                    queens |= bb;
                    continue;
                case 'r':
                case 'R':
                    rooks |= bb;
                    continue;
                case 'b':
                case 'B':
                    bishops |= bb;
                    continue;
                case 'n':
                case 'N':
                    knights |= bb;
                    continue;
                case 'p':
                case 'P':
                    pawns |= bb;
                    continue;

                default:
                    return false;
            }
        }

        char c = *fen++;
        if (!c) return false;
        if (rank > 0) {
            if (c != '/') return false;
        } else {
            if (c != ' ') return false;
        }
    }

    char c = *fen++;
    switch (c) {
        case 'w':
            turn = true;
            break;
        case 'b':
            turn = false;
            break;
        default:
            return false;
    }

    pos->white = white;
    pos->black = black;
    pos->kings = kings;
    pos->queens = queens;
    pos->rooks = rooks;
    pos->bishops = bishops;
    pos->knights = knights;
    pos->pawns = pawns;
    pos->turn = turn;

    return true;
}
