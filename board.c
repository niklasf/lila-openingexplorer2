#include <stdio.h>

#include "board.h"
#include "bitboard.h"
#include "square.h"
#include "attacks.h"

char board_piece_at(const struct board *pos, uint8_t square) {
    uint64_t bb = BB_SQUARE(square);

    bool white = pos->white & bb;
    bool black = pos->black & bb;

    if (white || black) {
        if (pos->kings & bb) return white ? 'K' : 'k';
        else if (pos->queens & bb) return white ? 'Q' : 'q';
        else if (pos->rooks & bb) return white ? 'R' : 'r';
        else if (pos->bishops & bb) return white ? 'B' : 'b';
        else if (pos->knights & bb) return white ? 'N' : 'n';
        else if (pos->pawns & bb) return white ? 'P' : 'p';
    }

    return 0;
}

void board_print(const struct board *pos) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            char piece = board_piece_at(pos, square(file, rank));
            printf("%c", piece ? piece : '.');

            if (file < 7) printf(" ");
            else printf("\n");
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

    pos->castling = 0;

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
    pos->castling = pos->rooks;

    pos->halfmove_clock = 0;
    pos->fullmove_number = 1;
}

char *board_board_fen(const struct board *pos, char *fen) {
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;

        for (int file = 0; file < 8; file++) {
            char piece = board_piece_at(pos, square(file, rank));

            if (piece) {
                if (empty) *fen++ = '0' + empty;
                *fen++ = piece;
                empty = 0;
            } else {
                empty++;
            }

            if (file == 7) {
                if (empty) *fen++ = '0' + empty;
                if (rank > 0) *fen++ = '/';
            }
        }
    }

    return fen;
}

char *board_castling_shredder_fen(const struct board *pos, char *castling_fen) {
    uint64_t white_castling = pos->castling & BB_RANK_1;
    uint64_t black_castling = pos->castling & BB_RANK_8;

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
    uint64_t kings = 0, queens = 0, rooks = 0, bishops = 0,
             knights = 0, pawns = 0;

    uint64_t castling = 0;
    bool turn = false;
    uint8_t ep_square = 0;

    int hmvc = 0, fmvn = 0;

    uint64_t promoted = 0;

    // 1. Board setup.

    for (int rank = 7; rank >= 0; rank--) {
        bool last_was_number = false;
        int file = 0;

        for (; file <= 7; file++) {
            uint64_t bb = BB_SQUARE(square(file, rank));
            char c = *fen++;

            if (c == '~') {
                promoted |= bb;
                c = *fen++;
            }

            if (c >= 'a' && c <= 'z') black |= bb;
            else if (c >= 'A' && c <= 'Z') white |= bb;
            else if (promoted && bb) return false;

            if (c >= '1' && c <= '8') {
                if (last_was_number) return false;
                file += c - '1';
                last_was_number = true;
                continue;
            } else {
                last_was_number = false;
            }

            if (c == 'k' || c == 'K') kings |= bb;
            else if (c == 'q' || c == 'Q') queens |= bb;
            else if (c == 'r' || c == 'R') rooks |= bb;
            else if (c == 'b' || c == 'B') bishops |= bb;
            else if (c == 'n' || c == 'N') knights |= bb;
            else if (c == 'p' || c == 'P') pawns |= bb;
            else return false;
        }

        if (file != 8) return false;

        char c = *fen++;
        if (!c) return false;

        if (rank > 0) {
            if (c != '/') return false;
        } else {
            if (c != ' ') return false;
        }
    }

    // 2. Turn.

    char c = *fen++;
    if (c == 'w') turn = true;
    else if (c == 'b') turn = false;
    else return false;
    if (*fen++ != ' ') return false;

    // 3. Castling

    c = *fen++;
    if (c != '-') {
        do {
            uint64_t backrank, king, candidates;
            if (c >= 'a' && c <= 'z') {
                backrank = BB_RANK_8;
                king = kings & black & BB_RANK_8;
                candidates = rooks & black & BB_RANK_8;
            } else {
                backrank = BB_RANK_1;
                king = kings & white & BB_RANK_1;
                candidates = rooks & white & BB_RANK_1;
            }

            if (c >= 'a' && c <= 'h') castling |= BB_SQUARE(square(c - 'a', 7));
            else if (c >= 'A' && c <= 'H') castling |= BB_SQUARE(square(c - 'A', 0));
            else if (c == 'q' || c == 'Q') {
                if (king && candidates && bb_lsb(candidates) < bb_lsb(king)) {
                    castling |= candidates & -candidates;
                } else {
                    castling |= BB_FILE_A & backrank;
                }
            } else if (c == 'k' || c == 'K') {
                if (king && candidates && bb_lsb(king) < bb_msb(candidates)) {
                    castling |= 1ULL << bb_msb(candidates);
                } else {
                    castling |= BB_FILE_H & backrank;
                }
            } else return false;
        } while ((c = *fen++) != ' ');
    } else if (*fen++ != ' ') return false;

    // 4. En-passant.

    c = *fen++;
    if (c != '-') {
        c = *fen++;
        if (c >= 'a' && c <= 'h') ep_square = c - 'a';
        else return false;

        c = *fen++;
        if (c == '3') ep_square += 2 * 8;
        else if (c == '6') ep_square += 5 * 8;
        else return false;
    }
    if (*fen++ != ' ') return false;

    // 5. Halfmove clock.

    c = *fen++;
    do {
        if (c >= '0' && c <= '9') hmvc = hmvc * 10 + c - '0';
        else return false;

        if (hmvc > 9999) hmvc = 9999;
    } while ((c = *fen++) != ' ');

    // 6. Fullmove number.

    c = *fen++;
    do {
        if (c >= '0' && c <= '9') fmvn = fmvn * 10 + c - '0';
        else return false;

        if (fmvn > 9999) fmvn = 9999;
    } while ((c = *fen++) && c != ' ');
    if (fmvn < 1) fmvn = 1;

    if (c) {
        return false;
    }

    // Commit board state.

    pos->white = white;
    pos->black = black;
    pos->kings = kings;
    pos->queens = queens;
    pos->rooks = rooks;
    pos->bishops = bishops;
    pos->knights = knights;
    pos->pawns = pawns;

    pos->turn = turn;
    pos->ep_square = ep_square;
    pos->castling = castling;

    pos->halfmove_clock = hmvc;
    pos->fullmove_number = fmvn;

    return true;
}

bool board_is_insufficient_material(const struct board *pos) {
    if (pos->pawns || pos->rooks || pos->queens) return false;
    else if (bb_popcount(pos->black | pos->white) <= 3) return true;
    else if (pos->knights) return false;
    else if (!(pos->bishops & BB_DARK_SQUARES)) return true;
    else if (!(pos->bishops & BB_LIGHT_SQUARES)) return true;
    else return false;
}

uint64_t board_attacks_to(const struct board *pos, uint8_t square) {
    uint64_t occupied = pos->white | pos->black;

    uint64_t attacks = 0;
    attacks |= attacks_rook(square, occupied) & (pos->rooks | pos->queens);
    attacks |= attacks_bishop(square, occupied) & (pos->bishops | pos->queens);
    attacks |= attacks_knight(square) & pos->knights;
    attacks |= attacks_king(square) & pos->kings;
    attacks |= attacks_pawn(square, true) & pos->pawns & pos->black;
    attacks |= attacks_pawn(square, false) & pos->pawns & pos->white;
    return attacks;
}

uint64_t board_attacks_from(const struct board *pos, uint8_t square) {
    uint64_t bb = BB_SQUARE(square);
    uint64_t occupied = pos->white | pos->black;
    if (pos->bishops & bb) return attacks_bishop(square, occupied);
    else if (pos->rooks & bb) return attacks_rook(square, occupied);
    else if (pos->queens & bb) return attacks_bishop(square, occupied) | attacks_rook(square, occupied);
    else if (pos->knights & bb) return attacks_knight(square);
    else if (pos->kings & bb) return attacks_king(square);
    else if (pos->pawns & bb) return attacks_pawn(square, (pos->black & bb) == 0);
    else return 0;
}

uint64_t board_checkers(const struct board *pos) {
    uint64_t we, them;
    if (pos->turn) {
        we = pos->white;
        them = pos->black;
    } else {
        we = pos->black;
        them = pos->white;
    }

    uint64_t king = we & pos->kings;
    if (!king) return 0;

    return board_attacks_to(pos, bb_lsb(king)) & them;
}
