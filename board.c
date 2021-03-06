#include <stdio.h>
#include <string.h>

#include "board.h"
#include "bitboard.h"
#include "square.h"
#include "attacks.h"

uint64_t board_pieces(const board_t *pos, piece_type_t piece_type, color_t color) {
    return pos->occupied[piece_type] & pos->occupied_co[color];
}

piece_type_t board_piece_type_at(const struct board *pos, uint8_t square) {
    return pos->pieces[square];
}

char board_piece_at(const struct board *pos, uint8_t square) {
    uint64_t bb = BB_SQUARE(square);
    piece_type_t pt = board_piece_type_at(pos, square);

    if (pos->occupied_co[0] & bb) return piece_symbol(pt, 0);
    if (pos->occupied_co[1] & bb) return piece_symbol(pt, 1);

    return 0;
}

void board_remove_piece_at(struct board *pos, uint8_t square) {
    uint64_t mask = ~BB_SQUARE(square);
    piece_type_t pt = pos->pieces[square];
    pos->pieces[square] = kNone;
    pos->occupied_co[0] &= mask;
    pos->occupied_co[1] &= mask;
    pos->occupied[pt] &= mask;
    pos->occupied[kAll] &= mask;
}

void board_set_piece_at(board_t *pos, square_t square, piece_type_t pt, color_t color) {
    board_remove_piece_at(pos, square);

    if (pt != kNone) {
        uint64_t bb = BB_SQUARE(square);
        pos->occupied_co[color] |= bb;
        pos->occupied[pt] |= bb;
        pos->occupied[kAll] |= bb;
        pos->pieces[square] = pt;
    }
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

void board_clear(board_t *pos) {
    memset(pos, 0, sizeof(board_t));

    pos->turn = kWhite;

    pos->castling = 0;

    pos->hmvc = 0;
    pos->fmvn = 1;
}

void board_reset(struct board *pos) {
    board_set_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
    uint64_t castling = board_castling_rights(pos);
    uint64_t white_castling = castling & BB_RANK_1;
    uint64_t black_castling = castling & BB_RANK_8;

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
    int hmvc = (pos->hmvc > 9999) ? 9999 : pos->hmvc;
    fen += sprintf(fen, "%d", hmvc);

    *fen++ = ' ';
    int fmvn = (pos->fmvn > 9999) ? 9999 : pos->fmvn;
    fen += sprintf(fen, "%d", fmvn);

    return fen;
}

bool board_set_fen(struct board *pos, const char *fen) {
    board_t updated;
    memset(&updated, 0, sizeof(board_t));

    uint64_t promoted = 0;

    // 1. Board setup.

    for (int rank = 7; rank >= 0; rank--) {
        bool last_was_number = false;
        int file = 0;

        for (; file <= 7; file++) {
            char c = *fen++;

            if (c == '~') {
                promoted |= BB_SQUARE(square(file, rank));
                c = *fen++;
            }

            color_t color;

            if (c >= 'a' && c <= 'z') color = kBlack;
            else if (c >= 'A' && c <= 'Z') color = kWhite;
            else if (promoted & BB_SQUARE(square(file, rank))) return false;

            if (c >= '1' && c <= '8') {
                if (last_was_number) return false;
                file += c - '1';
                last_was_number = true;
                continue;
            } else {
                last_was_number = false;
            }

            piece_type_t pt = piece_type_from_symbol(c);
            if (pt) board_set_piece_at(&updated, square(file, rank), pt, color);
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
    if (c == 'w') updated.turn = kWhite;
    else if (c == 'b') updated.turn = kBlack;
    else return false;
    if (*fen++ != ' ') return false;

    // 3. Castling
    c = *fen++;
    if (c != '-') {
        do {
            uint64_t backrank, king, candidates;
            if (c >= 'a' && c <= 'z') {
                backrank = BB_RANK_8;
                king = board_pieces(&updated, kKing, kBlack) & BB_RANK_8;
                candidates = board_pieces(&updated, kRook, kBlack) & BB_RANK_8;
            } else {
                backrank = BB_RANK_1;
                king = board_pieces(&updated, kKing, kWhite) & BB_RANK_1;
                candidates = board_pieces(&updated, kRook, kWhite) & BB_RANK_1;
            }

            if (c >= 'a' && c <= 'h') updated.castling |= BB_SQUARE(square(c - 'a', 7));
            else if (c >= 'A' && c <= 'H') updated.castling |= BB_SQUARE(square(c - 'A', 0));
            else if (c == 'q' || c == 'Q') {
                if (king && candidates && bb_lsb(candidates) < bb_lsb(king)) {
                    updated.castling |= candidates & -candidates;
                } else {
                    updated.castling |= BB_FILE_A & backrank;
                }
            } else if (c == 'k' || c == 'K') {
                if (king && candidates && bb_lsb(king) < bb_msb(candidates)) {
                    updated.castling |= 1ULL << bb_msb(candidates);
                } else {
                    updated.castling |= BB_FILE_H & backrank;
                }
            } else return false;
        } while ((c = *fen++) != ' ');
    } else if (*fen++ != ' ') return false;

    // 4. En-passant.

    c = *fen++;
    if (c != '-') {
        if (c >= 'a' && c <= 'h') updated.ep_square = c - 'a';
        else return false;

        c = *fen++;
        if (c == '3') updated.ep_square += 2 * 8;
        else if (c == '6') updated.ep_square += 5 * 8;
        else return false;
    }
    if (*fen++ != ' ') return false;

    // 5. Halfmove clock.

    c = *fen++;
    do {
        if (c >= '0' && c <= '9') updated.hmvc = updated.hmvc * 10 + c - '0';
        else return false;

        if (updated.hmvc > 9999) updated.hmvc = 9999;
    } while ((c = *fen++) != ' ');

    // 6. Fullmove number.

    c = *fen++;
    do {
        if (c >= '0' && c <= '9') updated.fmvn = updated.fmvn * 10 + c - '0';
        else return false;

        if (updated.fmvn > 9999) updated.fmvn = 9999;
    } while ((c = *fen++) && c != ' ');
    if (updated.fmvn < 1) updated.fmvn = 1;

    if (c) {
        return false;
    }

    // Commit board state.
    *pos = updated;
    return true;
}

bool board_is_insufficient_material(const struct board *pos) {
    if (pos->occupied[kPawn] || pos->occupied[kRook] || pos->occupied[kQueen]) return false;
    else if (bb_popcount(pos->occupied[kAll]) <= 3) return true;
    else if (pos->occupied[kKnight]) return false;
    else if (!(pos->occupied[kBishop] & BB_DARK_SQUARES)) return true;
    else if (!(pos->occupied[kBishop] & BB_LIGHT_SQUARES)) return true;
    else return false;
}

uint64_t board_attacks_to(const struct board *pos, uint8_t square) {
    uint64_t occupied = pos->occupied[kAll];

    uint64_t attacks = 0;
    attacks |= attacks_rook(square, occupied) & (pos->occupied[kRook] | pos->occupied[kQueen]);
    attacks |= attacks_bishop(square, occupied) & (pos->occupied[kBishop] | pos->occupied[kQueen]);
    attacks |= attacks_knight(square) & pos->occupied[kKnight];
    attacks |= attacks_king(square) & pos->occupied[kKing];
    attacks |= attacks_pawn(square, true) & pos->occupied[kPawn] & pos->occupied_co[0];
    attacks |= attacks_pawn(square, false) & pos->occupied[kPawn] & pos->occupied_co[1];
    return attacks;
}

uint64_t board_attacks_from(const struct board *pos, uint8_t square) {
    uint64_t bb = BB_SQUARE(square);
    uint64_t occupied = pos->occupied[kAll];
    switch (board_piece_type_at(pos, square)) {
        case kBishop: return attacks_bishop(square, occupied);
        case kRook: return attacks_rook(square, occupied);
        case kQueen: return attacks_bishop(square, occupied) | attacks_rook(square, occupied);
        case kKnight: return attacks_knight(square);
        case kKing: return attacks_king(square);
        case kPawn: return attacks_pawn(square, (pos->occupied_co[0] & bb) == 0);
        default: return 0;
    }
}

uint64_t board_checkers(const board_t *pos, color_t turn) {
    uint64_t king = board_pieces(pos, kKing, turn);
    if (!king) return 0;

    return board_attacks_to(pos, bb_lsb(king)) & pos->occupied_co[!turn];
}

bool board_is_checkmate(const struct board *pos) {
    if (!board_checkers(pos, pos->turn)) return false;

    move_t moves[255];
    move_t *end = board_legal_moves(pos, moves, BB_ALL, BB_ALL);
    return moves == end;
}

uint64_t board_castling_rights(const board_t *pos) {
    uint64_t castling = pos->castling & pos->occupied[kRook];
    uint64_t white_castling = castling & BB_RANK_1 & pos->occupied_co[1];
    uint64_t black_castling = castling & BB_RANK_8 & pos->occupied_co[0];

    // Find the kings.
    uint64_t white_king = pos->occupied_co[1] & pos->occupied[kKing] & BB_RANK_1;
    uint64_t black_king = pos->occupied_co[0] & pos->occupied[kKing] & BB_RANK_8;

    // The kings must be on the backrank.
    if (!white_king) white_castling = 0;
    if (!black_king) black_castling = 0;

    // Kings must be on the same file, giving preference to the e-file
    // and then to white.
    if (white_castling && black_castling) {
        if (square_file(bb_lsb(white_king)) != square_file(bb_lsb(black_king))) {
            if (square_file(bb_lsb(black_king)) == 4) white_castling = 0;
            else black_castling = 0;
        }
    }

    // There are only two ways of castling, a-side and h-side, and the king
    // must be between the rooks.
    uint64_t white_a_side = white_castling & -white_castling;
    uint64_t white_h_side = white_castling ? (1ULL << bb_msb(white_castling)) : 0;
    uint64_t black_a_side = black_castling & -black_castling;
    uint64_t black_h_side = black_castling ? (1ULL << bb_msb(black_castling)) : 0;
    if (white_a_side && bb_lsb(white_a_side) > bb_lsb(white_king)) white_a_side = 0;
    if (white_h_side && bb_lsb(white_h_side) < bb_lsb(white_king)) white_h_side = 0;
    if (black_a_side && bb_lsb(black_a_side) > bb_lsb(black_king)) black_a_side = 0;
    if (black_h_side && bb_lsb(black_h_side) < bb_lsb(black_king)) black_h_side = 0;

    // Rooks must be on the same file, giving preference to the a or h file
    // and then to white.
    if (black_a_side && white_a_side && square_file(bb_lsb(black_a_side)) != square_file(bb_lsb(white_a_side))) {
        if (black_a_side == BB_A8) white_a_side = 0;
        else black_a_side = 0;
    }
    if (black_h_side && white_h_side && square_file(bb_lsb(black_h_side)) != square_file(bb_lsb(white_h_side))) {
        if (black_h_side == BB_H8) white_h_side = 0;
        else black_h_side = 0;
    }

    // Done.
    return black_a_side | black_h_side | white_a_side | white_h_side;
}

move_t *board_castling_moves(const board_t *pos, move_t *moves, uint64_t from_mask, uint64_t to_mask) {
    uint64_t we = pos->occupied_co[pos->turn];
    uint64_t them = pos->occupied_co[!pos->turn];
    uint64_t backrank = pos->turn ? BB_RANK_1 : BB_RANK_8;

    uint64_t king_bb = we & pos->occupied[kKing] & from_mask & backrank;
    uint64_t candidates = board_castling_rights(pos) & backrank & to_mask;
    if (!king_bb || !candidates) return moves;
    square_t king = bb_lsb(king_bb);

    if (board_checkers(pos, pos->turn)) return moves;

    uint64_t bb_a = BB_FILE_A & backrank;
    uint64_t bb_c = BB_FILE_C & backrank;
    uint64_t bb_d = BB_FILE_D & backrank;
    uint64_t bb_f = BB_FILE_F & backrank;
    uint64_t bb_g = BB_FILE_G & backrank;

    do {
        uint64_t rook_bb = candidates & -candidates;
        square_t rook = bb_poplsb(&candidates);
        bool a_side = square_file(rook) < square_file(king);

        if (a_side && (rook_bb & BB_FILE_B) && (them & (pos->occupied[kQueen] | pos->occupied[kRook]) & bb_a)) {
            // We can't castle a-side because our rook shielded us from an
            // attack from a1 or a8.
            continue;
        }

        uint64_t empty_for_rook = 0, empty_for_king = 0;

        if (a_side) {
            if (!(rook_bb & bb_d)) {
                empty_for_rook =
                    attacks_rook(rook, rook_bb | bb_d) &
                    attacks_rook(bb_lsb(bb_d), rook_bb | bb_d);

                empty_for_rook |= bb_d;
            }

            if (!(king_bb & bb_c)) {
                empty_for_king =
                    attacks_rook(king, king_bb | bb_c) &
                    attacks_rook(bb_lsb(bb_c), king_bb | bb_c);

                empty_for_king |= bb_c;
            }
        } else {
            if (!(rook_bb & bb_f)) {
                empty_for_rook =
                    attacks_rook(rook, rook_bb | bb_f) &
                    attacks_rook(bb_lsb(bb_f), rook_bb | bb_f);

                empty_for_rook |= bb_f;
            }

            if (!(king_bb & bb_g)) {
                empty_for_king =
                    attacks_rook(king, king_bb | bb_g) &
                    attacks_rook(bb_lsb(bb_g), king_bb | bb_g);

                empty_for_king |= bb_g;
            }
        }

        empty_for_rook &= ~king_bb;
        empty_for_rook &= ~rook_bb;

        empty_for_king &= ~king_bb;
        uint64_t not_attacked_for_king = empty_for_king;
        empty_for_king &= ~rook_bb;

        if (pos->occupied[kAll] & (empty_for_king | empty_for_rook)) {
            continue;
        }

        bool any_attacked = false;
        while (not_attacked_for_king) {
            square_t test_square = bb_poplsb(&not_attacked_for_king);
            if (board_attacks_to(pos, test_square) & them) {
                any_attacked = true;
                break;
            }
        }
        if (any_attacked) continue;

        *moves++ = move_make(king, rook, 0);
    } while (candidates);

    return moves;
}

static move_t *make_pawn_moves(const board_t *pos, square_t from, square_t to, move_t *moves) {
    int rank = square_rank(to);
    if (rank == 0 || rank == 7) {
        *moves++ = move_make(from, to, kQueen);
        *moves++ = move_make(from, to, kRook);
        *moves++ = move_make(from, to, kBishop);
        *moves++ = move_make(from, to, kKnight);
    } else {
        *moves++ = move_make(from, to, kNone);
    }
    return moves;
}

move_t *board_pseudo_legal_moves(const board_t *pos, move_t *moves, uint64_t from_mask, uint64_t to_mask) {
    uint64_t we = pos->occupied_co[pos->turn];
    uint64_t them = pos->occupied_co[!pos->turn];

    // Generate piece moves.
    uint64_t non_pawns = we & ~pos->occupied[kPawn] & from_mask;
    while (non_pawns) {
        square_t from_square = bb_poplsb(&non_pawns);
        uint64_t to_squares = board_attacks_from(pos, from_square) & ~we & to_mask;
        while (to_squares) {
            square_t to_square = bb_poplsb(&to_squares);
            *moves++ = move_make(from_square, to_square, 0);
        }

    }

    // Generate castling moves.
    moves = board_castling_moves(pos, moves, from_mask, to_mask);

    // Generate pawn captures.
    uint64_t pawns = we & pos->occupied[kPawn] & from_mask;
    uint64_t ep_mask = pos->ep_square ? BB_SQUARE(pos->ep_square) : BB_VOID;
    while (pawns) {
        square_t from_square = bb_poplsb(&pawns);
        uint64_t to_squares = attacks_pawn(from_square, pos->turn) & (them | ep_mask) & to_mask;
        while (to_squares) {
            square_t to_square = bb_poplsb(&to_squares);
            moves = make_pawn_moves(pos, from_square, to_square, moves);
        }
    }

    // Prepare pawn advance generation.
    uint64_t single_moves, double_moves;
    if (pos->turn) {
        single_moves = ((we & pos->occupied[kPawn] & from_mask) << 8) & ~(we | them);
        double_moves = (single_moves << 8) & ~(we | them) & BB_RANK_4;
    } else {
        single_moves = ((we & pos->occupied[kPawn] & from_mask) >> 8) & ~(we | them);
        double_moves = (single_moves >> 8) & ~(we | them) & BB_RANK_5;
    }
    single_moves &= to_mask;
    double_moves &= to_mask;

    // Generate single pawn moves.
    while (single_moves) {
        square_t to_square = bb_poplsb(&single_moves);
        square_t from_square = to_square + (pos->turn ? -8 : 8);
        moves = make_pawn_moves(pos, from_square, to_square, moves);
    }

    // Generate double pawn moves.
    while (double_moves) {
        square_t to_square = bb_poplsb(&double_moves);
        square_t from_square = to_square + (pos->turn ? -16 : 16);
        moves = make_pawn_moves(pos, from_square, to_square, moves);
    }

    return moves;
}

void board_move(board_t *pos, move_t move) {
    uint64_t we = pos->occupied_co[pos->turn];
    uint64_t them = pos->occupied_co[!pos->turn];

    // Increment fullmove number.
    if (!pos->turn) pos->fmvn++;

    // On a null move simply swap turns and reset the en passant square.
    if (!move) {
        pos->turn = !pos->turn;
        pos->hmvc++;
        pos->ep_square = 0;
        return;
    }

    // Update the half move clock.
    piece_type_t piece = board_piece_type_at(pos, move_from(move));
    if (piece == kPawn || BB_SQUARE(move_to(move)) & them) {
        pos->hmvc = 0;
    } else {
        pos->hmvc++;
    }

    // Update castling rights.
    pos->castling = board_castling_rights(pos);
    pos->castling &= ~BB_SQUARE(move_to(move));
    pos->castling &= ~BB_SQUARE(move_from(move));
    if (piece == kKing) pos->castling &= ~(pos->turn ? BB_RANK_1 : BB_RANK_8);

    // Promotion.
    piece_type_t promotion = move_piece_type(move);

    // Remove piece from original square.
    board_remove_piece_at(pos, move_from(move));

    // Handle special pawn moves.
    pos->ep_square = 0;
    int diff = move_to(move) - move_from(move);
    if (piece == kPawn && pos->turn) {
        // Remove pawns captured en passant.
        if ((diff == 7 || diff == 9) && !(BB_SQUARE(move_to(move)) & them)) {
            board_remove_piece_at(pos, move_to(move) - 8);
        }

        // Set en passant square.
        if (diff == 16) pos->ep_square = move_to(move) - 8;
    } else if (piece == kPawn) {
        // Remove pawns captured en passant.
        if ((diff == -7 || diff == -9) && !(BB_SQUARE(move_to(move)) & them)) {
            board_remove_piece_at(pos, move_to(move) + 8);
        }

        // Set en passant square.
        if (diff == -16) pos->ep_square = move_to(move) + 8;
    }

    if (piece == kKing && (BB_SQUARE(move_to(move)) & we)) {
        // Castling.
        board_remove_piece_at(pos, move_from(move));
        board_remove_piece_at(pos, move_to(move));

        bool a_side = square_file(move_to(move)) < square_file(move_from(move));
        if (a_side) {
            board_set_piece_at(pos, pos->turn ? SQ_C1 : SQ_C8, piece, pos->turn);
            board_set_piece_at(pos, pos->turn ? SQ_D1 : SQ_D8, kRook, pos->turn);
        } else {
            board_set_piece_at(pos, pos->turn ? SQ_G1 : SQ_G8, piece, pos->turn);
            board_set_piece_at(pos, pos->turn ? SQ_F1 : SQ_F8, kRook, pos->turn);
        }
    } else {
        // Put piece on target square.
        if (promotion) board_set_piece_at(pos, move_to(move), promotion, pos->turn);
        else board_set_piece_at(pos, move_to(move), piece, pos->turn);
    }

    // Swap turn.
    pos->turn = !pos->turn;
}

move_t *board_legal_moves(const board_t *pos, move_t *moves, uint64_t from_mask, uint64_t to_mask) {
    board_t test_board;

    move_t *first = moves;
    move_t *last = board_pseudo_legal_moves(pos, moves, from_mask, to_mask);

    while (first < last) {
        test_board = *pos;
        board_move(&test_board, *first);

        if (!board_checkers(&test_board, pos->turn)) {
            *moves++ = *first;
        }

        first++;
    }

    return moves;
}

bool board_parse_san(const board_t *pos, const char *san, move_t *move) {
    // Null moves.
    if (strcmp("--", san) == 2) {
        *move = 0;
        return true;
    }

    // Optional TODO: Chess960 support and validation.
    if (*san == 'O') {
        if (strcmp("O-O", san) == 0 || strcmp("O-O+", san) == 0 || strcmp("O-O#", san) == 0) {
            // Kingside castling.
            *move = pos->turn ? move_make(SQ_E1, SQ_H1, 0) : move_make(SQ_E8, SQ_H8, 0);
            return true;
        } else if (strcmp("O-O-O", san) == 0 || strcmp("O-O-O+", san) == 0 || strcmp("O-O-O#", san) == 0) {
            // Queenside castling.
            *move = pos->turn ? move_make(SQ_E1, SQ_A1, 0) : move_make(SQ_E8, SQ_A8, 0);
            return true;
        } else {
            return false;
        }
    }

    // Select piece type.
    uint64_t from_mask;
    uint64_t to_mask = BB_ALL;
    switch (*san) {
        case 'K':
            from_mask = pos->occupied[kKing];
            san++;
            break;
        case 'Q':
            from_mask = pos->occupied[kQueen];
            san++;
            break;
        case 'R':
            from_mask = pos->occupied[kRook];
            san++;
            break;
        case 'B':
            from_mask = pos->occupied[kBishop];
            san++;
            break;
        case 'N':
            from_mask = pos->occupied[kKnight];
            san++;
            break;
        default:
            from_mask = pos->occupied[kPawn];
            break;
    }

    // Parse squares.
    char from_file = 0, from_rank = 0, to_file = 0, to_rank = 0;

    if (*san >= 'a' && *san <= 'h') {
        from_file = *san;
        san++;
    }

    if (*san >= '1' && *san <= '9') {
        from_rank = *san;
        san++;
    }

    if (*san == 'x') {
        uint64_t captures = pos->occupied_co[!pos->turn];
        if (pos->ep_square) captures |= BB_SQUARE(pos->ep_square);
        to_mask &= captures;
        san++;
    }

    if (*san >= 'a' && *san <= 'h') {
        to_file = *san++;

        if (*san >= '1' && *san <= '9') to_rank = *san++;
        else return false;
    } else {
        to_file = from_file;
        to_rank = from_rank;

        from_file = 0;
        from_rank = 0;
    }

    if (!to_file || !to_rank) return false;
    to_mask &= BB_SQUARE(square(to_file - 'a', to_rank - '1'));

    if (from_file) from_mask &= BB_FILE(from_file - 'a');
    if (from_rank) from_mask &= BB_RANK(from_rank - '1');

    piece_type_t promotion = kNone;
    if (*san == '=') {
        san++;
        promotion = piece_type_from_symbol(*san++);
        if (!promotion) return false;
    }

    // Ignore # and + at the end.
    if (*san == '#') san++;
    else if (*san == '+') san++;
    if (*san) return false;

    *move = 0;
    move_t moves[64];
    move_t *end = board_legal_moves(pos, moves, from_mask, to_mask);
    for (move_t *current = moves; current < end; current++) {
        if (move_piece_type(*current) == promotion) {
            if (*move) return false;
            else *move = *current;
        }
    }

    return *move != 0;
}

bool board_is_en_passant(const board_t *pos, move_t move) {
    int diff = abs(move_to(move) - move_from(move));
    if (diff != 7 && diff != 9) return false;
    if (!(pos->occupied[kPawn] & BB_SQUARE(move_from(move)))) return false;
    if (pos->occupied_co[!pos->turn] & BB_SQUARE(move_to(move))) return false;
    return true;
}

bool board_is_capture(const board_t *pos, move_t move) {
    uint64_t them = pos->occupied_co[!pos->turn];
    return BB_SQUARE(move_to(move)) & them || board_is_en_passant(pos, move);
}

bool board_is_castling(const board_t *pos, move_t move) {
    return BB_SQUARE(move_to(move)) & pos->occupied_co[pos->turn];
}

char *board_san(const board_t *pos, move_t move, char *san) {
    if (!move) {
        strcpy(san, "--");
        return san;
    }

    board_t pos_after = *pos;
    board_move(&pos_after, move);
    bool check = board_checkers(&pos_after, pos_after.turn);
    bool checkmate = check && board_is_checkmate(&pos_after);

    // Castling.
    if (board_is_castling(pos, move)) {
        *san++ = 'O';
        *san++ = '-';
        *san++ = 'O';

        if (move_to(move) < move_from(move)) {
            *san++ = '-';
            *san++ = 'O';
        }

        if (checkmate) *san++ = '#';
        else if (check) *san++ = '+';
        *san = 0;
        return san;
    }

    piece_type_t piece_type = board_piece_type_at(pos, move_from(move));
    if (piece_type != kPawn) {
        // Add piece type to SAN.
        *san++ = piece_symbol(piece_type, kWhite);

        // Get ambiguous move candidates: Not exactly the current move but to
        // the same square.
        uint64_t from_mask = board_pieces(pos, piece_type, pos->turn) & ~BB_SQUARE(move_from(move));
        uint64_t to_mask = BB_SQUARE(move_to(move));

        uint64_t others = 0;
        move_t moves[64];
        move_t *end = board_legal_moves(pos, moves, from_mask, to_mask);
        for (move_t *current = moves; current < end; current++) {
            others |= BB_SQUARE(move_from(*current));
        }

        // Disambiguate.
        if (others) {
            bool row = false, column = false;

            if (others & BB_RANK(square_rank(move_from(move)))) column = true;

            if (others & BB_FILE(square_file(move_from(move)))) row = true;
            else column = true;

            if (column) *san++ = 'a' + square_file(move_from(move));
            if (row) *san++ = '1' + square_rank(move_from(move));
        }
    }

    // Captures.
    if (board_is_capture(pos, move)) {
        if (piece_type == kPawn) *san++ = 'a' + square_file(move_from(move));
        *san++ = 'x';
    }

    // Destination square.
    san = square_name(move_to(move), san);

    // Promotion.
    if (move_piece_type(move)) {
        *san++ = '=';
        *san++ = piece_symbol(move_piece_type(move), kWhite);
    }

    // Add check or checkmate suffix.
    if (checkmate) *san++ = '#';
    else if (check) *san++ = '+';
    *san = 0;
    return san;
}

move_t board_legal_en_passant(const board_t *pos) {
    if (!pos->ep_square) return 0;

    move_t moves[16];
    move_t *end = board_legal_moves(pos, moves, pos->occupied[kPawn], BB_SQUARE(pos->ep_square));
    for (move_t *current = moves; current < end; current++) {
        return *current;
    }

    return 0;
}

uint64_t board_zobrist_hash(const board_t *pos, const uint64_t array[]) {
    uint64_t zobrist_hash = 0;

    // Board setup.
    for (piece_type_t pt = kPawn; pt <= kKing; pt++) {
        uint64_t squares = pos->occupied[pt];
        while (squares) {
            square_t square = bb_poplsb(&squares);
            bool color = (pos->occupied_co[0] & BB_SQUARE(square)) == 0;
            int piece_index = (pt - 1) * 2 + color;
            zobrist_hash ^= array[64 * piece_index + 8 * square_rank(square) + square_file(square)];
        }
    }

    // Optional TODO: Chess960 Castling.
    uint64_t castling = board_castling_rights(pos);
    if (castling & BB_H1) zobrist_hash ^= array[768];
    if (castling & BB_A1) zobrist_hash ^= array[768 + 1];
    if (castling & BB_H8) zobrist_hash ^= array[768 + 2];
    if (castling & BB_A8) zobrist_hash ^= array[768 + 3];

    // En passant
    move_t ep = board_legal_en_passant(pos);
    if (ep) zobrist_hash ^= array[772 + square_file(move_to(ep))];

    // Turn.
    if (pos->turn) zobrist_hash ^= array[780];

    return zobrist_hash;
}

const uint64_t POLYGLOT[] = {
    0x9d39247e33776d41ULL, 0x2af7398005aaa5c7ULL, 0x44db015024623547ULL,
    0x9c15f73e62a76ae2ULL, 0x75834465489c0c89ULL, 0x3290ac3a203001bfULL,
    0x0fbbad1f61042279ULL, 0xe83a908ff2fb60caULL, 0x0d7e765d58755c10ULL,
    0x1a083822ceafe02dULL, 0x9605d5f0e25ec3b0ULL, 0xd021ff5cd13a2ed5ULL,
    0x40bdf15d4a672e32ULL, 0x011355146fd56395ULL, 0x5db4832046f3d9e5ULL,
    0x239f8b2d7ff719ccULL, 0x05d1a1ae85b49aa1ULL, 0x679f848f6e8fc971ULL,
    0x7449bbff801fed0bULL, 0x7d11cdb1c3b7adf0ULL, 0x82c7709e781eb7ccULL,
    0xf3218f1c9510786cULL, 0x331478f3af51bbe6ULL, 0x4bb38de5e7219443ULL,
    0xaa649c6ebcfd50fcULL, 0x8dbd98a352afd40bULL, 0x87d2074b81d79217ULL,
    0x19f3c751d3e92ae1ULL, 0xb4ab30f062b19abfULL, 0x7b0500ac42047ac4ULL,
    0xc9452ca81a09d85dULL, 0x24aa6c514da27500ULL, 0x4c9f34427501b447ULL,
    0x14a68fd73c910841ULL, 0xa71b9b83461cbd93ULL, 0x03488b95b0f1850fULL,
    0x637b2b34ff93c040ULL, 0x09d1bc9a3dd90a94ULL, 0x3575668334a1dd3bULL,
    0x735e2b97a4c45a23ULL, 0x18727070f1bd400bULL, 0x1fcbacd259bf02e7ULL,
    0xd310a7c2ce9b6555ULL, 0xbf983fe0fe5d8244ULL, 0x9f74d14f7454a824ULL,
    0x51ebdc4ab9ba3035ULL, 0x5c82c505db9ab0faULL, 0xfcf7fe8a3430b241ULL,
    0x3253a729b9ba3ddeULL, 0x8c74c368081b3075ULL, 0xb9bc6c87167c33e7ULL,
    0x7ef48f2b83024e20ULL, 0x11d505d4c351bd7fULL, 0x6568fca92c76a243ULL,
    0x4de0b0f40f32a7b8ULL, 0x96d693460cc37e5dULL, 0x42e240cb63689f2fULL,
    0x6d2bdcdae2919661ULL, 0x42880b0236e4d951ULL, 0x5f0f4a5898171bb6ULL,
    0x39f890f579f92f88ULL, 0x93c5b5f47356388bULL, 0x63dc359d8d231b78ULL,
    0xec16ca8aea98ad76ULL, 0x5355f900c2a82dc7ULL, 0x07fb9f855a997142ULL,
    0x5093417aa8a7ed5eULL, 0x7bcbc38da25a7f3cULL, 0x19fc8a768cf4b6d4ULL,
    0x637a7780decfc0d9ULL, 0x8249a47aee0e41f7ULL, 0x79ad695501e7d1e8ULL,
    0x14acbaf4777d5776ULL, 0xf145b6beccdea195ULL, 0xdabf2ac8201752fcULL,
    0x24c3c94df9c8d3f6ULL, 0xbb6e2924f03912eaULL, 0x0ce26c0b95c980d9ULL,
    0xa49cd132bfbf7cc4ULL, 0xe99d662af4243939ULL, 0x27e6ad7891165c3fULL,
    0x8535f040b9744ff1ULL, 0x54b3f4fa5f40d873ULL, 0x72b12c32127fed2bULL,
    0xee954d3c7b411f47ULL, 0x9a85ac909a24eaa1ULL, 0x70ac4cd9f04f21f5ULL,
    0xf9b89d3e99a075c2ULL, 0x87b3e2b2b5c907b1ULL, 0xa366e5b8c54f48b8ULL,
    0xae4a9346cc3f7cf2ULL, 0x1920c04d47267bbdULL, 0x87bf02c6b49e2ae9ULL,
    0x092237ac237f3859ULL, 0xff07f64ef8ed14d0ULL, 0x8de8dca9f03cc54eULL,
    0x9c1633264db49c89ULL, 0xb3f22c3d0b0b38edULL, 0x390e5fb44d01144bULL,
    0x5bfea5b4712768e9ULL, 0x1e1032911fa78984ULL, 0x9a74acb964e78cb3ULL,
    0x4f80f7a035dafb04ULL, 0x6304d09a0b3738c4ULL, 0x2171e64683023a08ULL,
    0x5b9b63eb9ceff80cULL, 0x506aacf489889342ULL, 0x1881afc9a3a701d6ULL,
    0x6503080440750644ULL, 0xdfd395339cdbf4a7ULL, 0xef927dbcf00c20f2ULL,
    0x7b32f7d1e03680ecULL, 0xb9fd7620e7316243ULL, 0x05a7e8a57db91b77ULL,
    0xb5889c6e15630a75ULL, 0x4a750a09ce9573f7ULL, 0xcf464cec899a2f8aULL,
    0xf538639ce705b824ULL, 0x3c79a0ff5580ef7fULL, 0xede6c87f8477609dULL,
    0x799e81f05bc93f31ULL, 0x86536b8cf3428a8cULL, 0x97d7374c60087b73ULL,
    0xa246637cff328532ULL, 0x043fcae60cc0eba0ULL, 0x920e449535dd359eULL,
    0x70eb093b15b290ccULL, 0x73a1921916591cbdULL, 0x56436c9fe1a1aa8dULL,
    0xefac4b70633b8f81ULL, 0xbb215798d45df7afULL, 0x45f20042f24f1768ULL,
    0x930f80f4e8eb7462ULL, 0xff6712ffcfd75ea1ULL, 0xae623fd67468aa70ULL,
    0xdd2c5bc84bc8d8fcULL, 0x7eed120d54cf2dd9ULL, 0x22fe545401165f1cULL,
    0xc91800e98fb99929ULL, 0x808bd68e6ac10365ULL, 0xdec468145b7605f6ULL,
    0x1bede3a3aef53302ULL, 0x43539603d6c55602ULL, 0xaa969b5c691ccb7aULL,
    0xa87832d392efee56ULL, 0x65942c7b3c7e11aeULL, 0xded2d633cad004f6ULL,
    0x21f08570f420e565ULL, 0xb415938d7da94e3cULL, 0x91b859e59ecb6350ULL,
    0x10cff333e0ed804aULL, 0x28aed140be0bb7ddULL, 0xc5cc1d89724fa456ULL,
    0x5648f680f11a2741ULL, 0x2d255069f0b7dab3ULL, 0x9bc5a38ef729abd4ULL,
    0xef2f054308f6a2bcULL, 0xaf2042f5cc5c2858ULL, 0x480412bab7f5be2aULL,
    0xaef3af4a563dfe43ULL, 0x19afe59ae451497fULL, 0x52593803dff1e840ULL,
    0xf4f076e65f2ce6f0ULL, 0x11379625747d5af3ULL, 0xbce5d2248682c115ULL,
    0x9da4243de836994fULL, 0x066f70b33fe09017ULL, 0x4dc4de189b671a1cULL,
    0x51039ab7712457c3ULL, 0xc07a3f80c31fb4b4ULL, 0xb46ee9c5e64a6e7cULL,
    0xb3819a42abe61c87ULL, 0x21a007933a522a20ULL, 0x2df16f761598aa4fULL,
    0x763c4a1371b368fdULL, 0xf793c46702e086a0ULL, 0xd7288e012aeb8d31ULL,
    0xde336a2a4bc1c44bULL, 0x0bf692b38d079f23ULL, 0x2c604a7a177326b3ULL,
    0x4850e73e03eb6064ULL, 0xcfc447f1e53c8e1bULL, 0xb05ca3f564268d99ULL,
    0x9ae182c8bc9474e8ULL, 0xa4fc4bd4fc5558caULL, 0xe755178d58fc4e76ULL,
    0x69b97db1a4c03dfeULL, 0xf9b5b7c4acc67c96ULL, 0xfc6a82d64b8655fbULL,
    0x9c684cb6c4d24417ULL, 0x8ec97d2917456ed0ULL, 0x6703df9d2924e97eULL,
    0xc547f57e42a7444eULL, 0x78e37644e7cad29eULL, 0xfe9a44e9362f05faULL,
    0x08bd35cc38336615ULL, 0x9315e5eb3a129aceULL, 0x94061b871e04df75ULL,
    0xdf1d9f9d784ba010ULL, 0x3bba57b68871b59dULL, 0xd2b7adeeded1f73fULL,
    0xf7a255d83bc373f8ULL, 0xd7f4f2448c0ceb81ULL, 0xd95be88cd210ffa7ULL,
    0x336f52f8ff4728e7ULL, 0xa74049dac312ac71ULL, 0xa2f61bb6e437fdb5ULL,
    0x4f2a5cb07f6a35b3ULL, 0x87d380bda5bf7859ULL, 0x16b9f7e06c453a21ULL,
    0x7ba2484c8a0fd54eULL, 0xf3a678cad9a2e38cULL, 0x39b0bf7dde437ba2ULL,
    0xfcaf55c1bf8a4424ULL, 0x18fcf680573fa594ULL, 0x4c0563b89f495ac3ULL,
    0x40e087931a00930dULL, 0x8cffa9412eb642c1ULL, 0x68ca39053261169fULL,
    0x7a1ee967d27579e2ULL, 0x9d1d60e5076f5b6fULL, 0x3810e399b6f65ba2ULL,
    0x32095b6d4ab5f9b1ULL, 0x35cab62109dd038aULL, 0xa90b24499fcfafb1ULL,
    0x77a225a07cc2c6bdULL, 0x513e5e634c70e331ULL, 0x4361c0ca3f692f12ULL,
    0xd941aca44b20a45bULL, 0x528f7c8602c5807bULL, 0x52ab92beb9613989ULL,
    0x9d1dfa2efc557f73ULL, 0x722ff175f572c348ULL, 0x1d1260a51107fe97ULL,
    0x7a249a57ec0c9ba2ULL, 0x04208fe9e8f7f2d6ULL, 0x5a110c6058b920a0ULL,
    0x0cd9a497658a5698ULL, 0x56fd23c8f9715a4cULL, 0x284c847b9d887aaeULL,
    0x04feabfbbdb619cbULL, 0x742e1e651c60ba83ULL, 0x9a9632e65904ad3cULL,
    0x881b82a13b51b9e2ULL, 0x506e6744cd974924ULL, 0xb0183db56ffc6a79ULL,
    0x0ed9b915c66ed37eULL, 0x5e11e86d5873d484ULL, 0xf678647e3519ac6eULL,
    0x1b85d488d0f20cc5ULL, 0xdab9fe6525d89021ULL, 0x0d151d86adb73615ULL,
    0xa865a54edcc0f019ULL, 0x93c42566aef98ffbULL, 0x99e7afeabe000731ULL,
    0x48cbff086ddf285aULL, 0x7f9b6af1ebf78bafULL, 0x58627e1a149bba21ULL,
    0x2cd16e2abd791e33ULL, 0xd363eff5f0977996ULL, 0x0ce2a38c344a6eedULL,
    0x1a804aadb9cfa741ULL, 0x907f30421d78c5deULL, 0x501f65edb3034d07ULL,
    0x37624ae5a48fa6e9ULL, 0x957baf61700cff4eULL, 0x3a6c27934e31188aULL,
    0xd49503536abca345ULL, 0x088e049589c432e0ULL, 0xf943aee7febf21b8ULL,
    0x6c3b8e3e336139d3ULL, 0x364f6ffa464ee52eULL, 0xd60f6dcedc314222ULL,
    0x56963b0dca418fc0ULL, 0x16f50edf91e513afULL, 0xef1955914b609f93ULL,
    0x565601c0364e3228ULL, 0xecb53939887e8175ULL, 0xbac7a9a18531294bULL,
    0xb344c470397bba52ULL, 0x65d34954daf3cebdULL, 0xb4b81b3fa97511e2ULL,
    0xb422061193d6f6a7ULL, 0x071582401c38434dULL, 0x7a13f18bbedc4ff5ULL,
    0xbc4097b116c524d2ULL, 0x59b97885e2f2ea28ULL, 0x99170a5dc3115544ULL,
    0x6f423357e7c6a9f9ULL, 0x325928ee6e6f8794ULL, 0xd0e4366228b03343ULL,
    0x565c31f7de89ea27ULL, 0x30f5611484119414ULL, 0xd873db391292ed4fULL,
    0x7bd94e1d8e17debcULL, 0xc7d9f16864a76e94ULL, 0x947ae053ee56e63cULL,
    0xc8c93882f9475f5fULL, 0x3a9bf55ba91f81caULL, 0xd9a11fbb3d9808e4ULL,
    0x0fd22063edc29fcaULL, 0xb3f256d8aca0b0b9ULL, 0xb03031a8b4516e84ULL,
    0x35dd37d5871448afULL, 0xe9f6082b05542e4eULL, 0xebfafa33d7254b59ULL,
    0x9255abb50d532280ULL, 0xb9ab4ce57f2d34f3ULL, 0x693501d628297551ULL,
    0xc62c58f97dd949bfULL, 0xcd454f8f19c5126aULL, 0xbbe83f4ecc2bdecbULL,
    0xdc842b7e2819e230ULL, 0xba89142e007503b8ULL, 0xa3bc941d0a5061cbULL,
    0xe9f6760e32cd8021ULL, 0x09c7e552bc76492fULL, 0x852f54934da55cc9ULL,
    0x8107fccf064fcf56ULL, 0x098954d51fff6580ULL, 0x23b70edb1955c4bfULL,
    0xc330de426430f69dULL, 0x4715ed43e8a45c0aULL, 0xa8d7e4dab780a08dULL,
    0x0572b974f03ce0bbULL, 0xb57d2e985e1419c7ULL, 0xe8d9ecbe2cf3d73fULL,
    0x2fe4b17170e59750ULL, 0x11317ba87905e790ULL, 0x7fbf21ec8a1f45ecULL,
    0x1725cabfcb045b00ULL, 0x964e915cd5e2b207ULL, 0x3e2b8bcbf016d66dULL,
    0xbe7444e39328a0acULL, 0xf85b2b4fbcde44b7ULL, 0x49353fea39ba63b1ULL,
    0x1dd01aafcd53486aULL, 0x1fca8a92fd719f85ULL, 0xfc7c95d827357afaULL,
    0x18a6a990c8b35ebdULL, 0xcccb7005c6b9c28dULL, 0x3bdbb92c43b17f26ULL,
    0xaa70b5b4f89695a2ULL, 0xe94c39a54a98307fULL, 0xb7a0b174cff6f36eULL,
    0xd4dba84729af48adULL, 0x2e18bc1ad9704a68ULL, 0x2de0966daf2f8b1cULL,
    0xb9c11d5b1e43a07eULL, 0x64972d68dee33360ULL, 0x94628d38d0c20584ULL,
    0xdbc0d2b6ab90a559ULL, 0xd2733c4335c6a72fULL, 0x7e75d99d94a70f4dULL,
    0x6ced1983376fa72bULL, 0x97fcaacbf030bc24ULL, 0x7b77497b32503b12ULL,
    0x8547eddfb81ccb94ULL, 0x79999cdff70902cbULL, 0xcffe1939438e9b24ULL,
    0x829626e3892d95d7ULL, 0x92fae24291f2b3f1ULL, 0x63e22c147b9c3403ULL,
    0xc678b6d860284a1cULL, 0x5873888850659ae7ULL, 0x0981dcd296a8736dULL,
    0x9f65789a6509a440ULL, 0x9ff38fed72e9052fULL, 0xe479ee5b9930578cULL,
    0xe7f28ecd2d49eecdULL, 0x56c074a581ea17feULL, 0x5544f7d774b14aefULL,
    0x7b3f0195fc6f290fULL, 0x12153635b2c0cf57ULL, 0x7f5126dbba5e0ca7ULL,
    0x7a76956c3eafb413ULL, 0x3d5774a11d31ab39ULL, 0x8a1b083821f40cb4ULL,
    0x7b4a38e32537df62ULL, 0x950113646d1d6e03ULL, 0x4da8979a0041e8a9ULL,
    0x3bc36e078f7515d7ULL, 0x5d0a12f27ad310d1ULL, 0x7f9d1a2e1ebe1327ULL,
    0xda3a361b1c5157b1ULL, 0xdcdd7d20903d0c25ULL, 0x36833336d068f707ULL,
    0xce68341f79893389ULL, 0xab9090168dd05f34ULL, 0x43954b3252dc25e5ULL,
    0xb438c2b67f98e5e9ULL, 0x10dcd78e3851a492ULL, 0xdbc27ab5447822bfULL,
    0x9b3cdb65f82ca382ULL, 0xb67b7896167b4c84ULL, 0xbfced1b0048eac50ULL,
    0xa9119b60369ffebdULL, 0x1fff7ac80904bf45ULL, 0xac12fb171817eee7ULL,
    0xaf08da9177dda93dULL, 0x1b0cab936e65c744ULL, 0xb559eb1d04e5e932ULL,
    0xc37b45b3f8d6f2baULL, 0xc3a9dc228caac9e9ULL, 0xf3b8b6675a6507ffULL,
    0x9fc477de4ed681daULL, 0x67378d8eccef96cbULL, 0x6dd856d94d259236ULL,
    0xa319ce15b0b4db31ULL, 0x073973751f12dd5eULL, 0x8a8e849eb32781a5ULL,
    0xe1925c71285279f5ULL, 0x74c04bf1790c0efeULL, 0x4dda48153c94938aULL,
    0x9d266d6a1cc0542cULL, 0x7440fb816508c4feULL, 0x13328503df48229fULL,
    0xd6bf7baee43cac40ULL, 0x4838d65f6ef6748fULL, 0x1e152328f3318deaULL,
    0x8f8419a348f296bfULL, 0x72c8834a5957b511ULL, 0xd7a023a73260b45cULL,
    0x94ebc8abcfb56daeULL, 0x9fc10d0f989993e0ULL, 0xde68a2355b93cae6ULL,
    0xa44cfe79ae538bbeULL, 0x9d1d84fcce371425ULL, 0x51d2b1ab2ddfb636ULL,
    0x2fd7e4b9e72cd38cULL, 0x65ca5b96b7552210ULL, 0xdd69a0d8ab3b546dULL,
    0x604d51b25fbf70e2ULL, 0x73aa8a564fb7ac9eULL, 0x1a8c1e992b941148ULL,
    0xaac40a2703d9bea0ULL, 0x764dbeae7fa4f3a6ULL, 0x1e99b96e70a9be8bULL,
    0x2c5e9deb57ef4743ULL, 0x3a938fee32d29981ULL, 0x26e6db8ffdf5adfeULL,
    0x469356c504ec9f9dULL, 0xc8763c5b08d1908cULL, 0x3f6c6af859d80055ULL,
    0x7f7cc39420a3a545ULL, 0x9bfb227ebdf4c5ceULL, 0x89039d79d6fc5c5cULL,
    0x8fe88b57305e2ab6ULL, 0xa09e8c8c35ab96deULL, 0xfa7e393983325753ULL,
    0xd6b6d0ecc617c699ULL, 0xdfea21ea9e7557e3ULL, 0xb67c1fa481680af8ULL,
    0xca1e3785a9e724e5ULL, 0x1cfc8bed0d681639ULL, 0xd18d8549d140caeaULL,
    0x4ed0fe7e9dc91335ULL, 0xe4dbf0634473f5d2ULL, 0x1761f93a44d5aefeULL,
    0x53898e4c3910da55ULL, 0x734de8181f6ec39aULL, 0x2680b122baa28d97ULL,
    0x298af231c85bafabULL, 0x7983eed3740847d5ULL, 0x66c1a2a1a60cd889ULL,
    0x9e17e49642a3e4c1ULL, 0xedb454e7badc0805ULL, 0x50b704cab602c329ULL,
    0x4cc317fb9cddd023ULL, 0x66b4835d9eafea22ULL, 0x219b97e26ffc81bdULL,
    0x261e4e4c0a333a9dULL, 0x1fe2cca76517db90ULL, 0xd7504dfa8816edbbULL,
    0xb9571fa04dc089c8ULL, 0x1ddc0325259b27deULL, 0xcf3f4688801eb9aaULL,
    0xf4f5d05c10cab243ULL, 0x38b6525c21a42b0eULL, 0x36f60e2ba4fa6800ULL,
    0xeb3593803173e0ceULL, 0x9c4cd6257c5a3603ULL, 0xaf0c317d32adaa8aULL,
    0x258e5a80c7204c4bULL, 0x8b889d624d44885dULL, 0xf4d14597e660f855ULL,
    0xd4347f66ec8941c3ULL, 0xe699ed85b0dfb40dULL, 0x2472f6207c2d0484ULL,
    0xc2a1e7b5b459aeb5ULL, 0xab4f6451cc1d45ecULL, 0x63767572ae3d6174ULL,
    0xa59e0bd101731a28ULL, 0x116d0016cb948f09ULL, 0x2cf9c8ca052f6e9fULL,
    0x0b090a7560a968e3ULL, 0xabeeddb2dde06ff1ULL, 0x58efc10b06a2068dULL,
    0xc6e57a78fbd986e0ULL, 0x2eab8ca63ce802d7ULL, 0x14a195640116f336ULL,
    0x7c0828dd624ec390ULL, 0xd74bbe77e6116ac7ULL, 0x804456af10f5fb53ULL,
    0xebe9ea2adf4321c7ULL, 0x03219a39ee587a30ULL, 0x49787fef17af9924ULL,
    0xa1e9300cd8520548ULL, 0x5b45e522e4b1b4efULL, 0xb49c3b3995091a36ULL,
    0xd4490ad526f14431ULL, 0x12a8f216af9418c2ULL, 0x001f837cc7350524ULL,
    0x1877b51e57a764d5ULL, 0xa2853b80f17f58eeULL, 0x993e1de72d36d310ULL,
    0xb3598080ce64a656ULL, 0x252f59cf0d9f04bbULL, 0xd23c8e176d113600ULL,
    0x1bda0492e7e4586eULL, 0x21e0bd5026c619bfULL, 0x3b097adaf088f94eULL,
    0x8d14dedb30be846eULL, 0xf95cffa23af5f6f4ULL, 0x3871700761b3f743ULL,
    0xca672b91e9e4fa16ULL, 0x64c8e531bff53b55ULL, 0x241260ed4ad1e87dULL,
    0x106c09b972d2e822ULL, 0x7fba195410e5ca30ULL, 0x7884d9bc6cb569d8ULL,
    0x0647dfedcd894a29ULL, 0x63573ff03e224774ULL, 0x4fc8e9560f91b123ULL,
    0x1db956e450275779ULL, 0xb8d91274b9e9d4fbULL, 0xa2ebee47e2fbfce1ULL,
    0xd9f1f30ccd97fb09ULL, 0xefed53d75fd64e6bULL, 0x2e6d02c36017f67fULL,
    0xa9aa4d20db084e9bULL, 0xb64be8d8b25396c1ULL, 0x70cb6af7c2d5bcf0ULL,
    0x98f076a4f7a2322eULL, 0xbf84470805e69b5fULL, 0x94c3251f06f90cf3ULL,
    0x3e003e616a6591e9ULL, 0xb925a6cd0421aff3ULL, 0x61bdd1307c66e300ULL,
    0xbf8d5108e27e0d48ULL, 0x240ab57a8b888b20ULL, 0xfc87614baf287e07ULL,
    0xef02cdd06ffdb432ULL, 0xa1082c0466df6c0aULL, 0x8215e577001332c8ULL,
    0xd39bb9c3a48db6cfULL, 0x2738259634305c14ULL, 0x61cf4f94c97df93dULL,
    0x1b6baca2ae4e125bULL, 0x758f450c88572e0bULL, 0x959f587d507a8359ULL,
    0xb063e962e045f54dULL, 0x60e8ed72c0dff5d1ULL, 0x7b64978555326f9fULL,
    0xfd080d236da814baULL, 0x8c90fd9b083f4558ULL, 0x106f72fe81e2c590ULL,
    0x7976033a39f7d952ULL, 0xa4ec0132764ca04bULL, 0x733ea705fae4fa77ULL,
    0xb4d8f77bc3e56167ULL, 0x9e21f4f903b33fd9ULL, 0x9d765e419fb69f6dULL,
    0xd30c088ba61ea5efULL, 0x5d94337fbfaf7f5bULL, 0x1a4e4822eb4d7a59ULL,
    0x6ffe73e81b637fb3ULL, 0xddf957bc36d8b9caULL, 0x64d0e29eea8838b3ULL,
    0x08dd9bdfd96b9f63ULL, 0x087e79e5a57d1d13ULL, 0xe328e230e3e2b3fbULL,
    0x1c2559e30f0946beULL, 0x720bf5f26f4d2eaaULL, 0xb0774d261cc609dbULL,
    0x443f64ec5a371195ULL, 0x4112cf68649a260eULL, 0xd813f2fab7f5c5caULL,
    0x660d3257380841eeULL, 0x59ac2c7873f910a3ULL, 0xe846963877671a17ULL,
    0x93b633abfa3469f8ULL, 0xc0c0f5a60ef4cdcfULL, 0xcaf21ecd4377b28cULL,
    0x57277707199b8175ULL, 0x506c11b9d90e8b1dULL, 0xd83cc2687a19255fULL,
    0x4a29c6465a314cd1ULL, 0xed2df21216235097ULL, 0xb5635c95ff7296e2ULL,
    0x22af003ab672e811ULL, 0x52e762596bf68235ULL, 0x9aeba33ac6ecc6b0ULL,
    0x944f6de09134dfb6ULL, 0x6c47bec883a7de39ULL, 0x6ad047c430a12104ULL,
    0xa5b1cfdba0ab4067ULL, 0x7c45d833aff07862ULL, 0x5092ef950a16da0bULL,
    0x9338e69c052b8e7bULL, 0x455a4b4cfe30e3f5ULL, 0x6b02e63195ad0cf8ULL,
    0x6b17b224bad6bf27ULL, 0xd1e0ccd25bb9c169ULL, 0xde0c89a556b9ae70ULL,
    0x50065e535a213cf6ULL, 0x9c1169fa2777b874ULL, 0x78edefd694af1eedULL,
    0x6dc93d9526a50e68ULL, 0xee97f453f06791edULL, 0x32ab0edb696703d3ULL,
    0x3a6853c7e70757a7ULL, 0x31865ced6120f37dULL, 0x67fef95d92607890ULL,
    0x1f2b1d1f15f6dc9cULL, 0xb69e38a8965c6b65ULL, 0xaa9119ff184cccf4ULL,
    0xf43c732873f24c13ULL, 0xfb4a3d794a9a80d2ULL, 0x3550c2321fd6109cULL,
    0x371f77e76bb8417eULL, 0x6bfa9aae5ec05779ULL, 0xcd04f3ff001a4778ULL,
    0xe3273522064480caULL, 0x9f91508bffcfc14aULL, 0x049a7f41061a9e60ULL,
    0xfcb6be43a9f2fe9bULL, 0x08de8a1c7797da9bULL, 0x8f9887e6078735a1ULL,
    0xb5b4071dbfc73a66ULL, 0x230e343dfba08d33ULL, 0x43ed7f5a0fae657dULL,
    0x3a88a0fbbcb05c63ULL, 0x21874b8b4d2dbc4fULL, 0x1bdea12e35f6a8c9ULL,
    0x53c065c6c8e63528ULL, 0xe34a1d250e7a8d6bULL, 0xd6b04d3b7651dd7eULL,
    0x5e90277e7cb39e2dULL, 0x2c046f22062dc67dULL, 0xb10bb459132d0a26ULL,
    0x3fa9ddfb67e2f199ULL, 0x0e09b88e1914f7afULL, 0x10e8b35af3eeab37ULL,
    0x9eedeca8e272b933ULL, 0xd4c718bc4ae8ae5fULL, 0x81536d601170fc20ULL,
    0x91b534f885818a06ULL, 0xec8177f83f900978ULL, 0x190e714fada5156eULL,
    0xb592bf39b0364963ULL, 0x89c350c893ae7dc1ULL, 0xac042e70f8b383f2ULL,
    0xb49b52e587a1ee60ULL, 0xfb152fe3ff26da89ULL, 0x3e666e6f69ae2c15ULL,
    0x3b544ebe544c19f9ULL, 0xe805a1e290cf2456ULL, 0x24b33c9d7ed25117ULL,
    0xe74733427b72f0c1ULL, 0x0a804d18b7097475ULL, 0x57e3306d881edb4fULL,
    0x4ae7d6a36eb5dbcbULL, 0x2d8d5432157064c8ULL, 0xd1e649de1e7f268bULL,
    0x8a328a1cedfe552cULL, 0x07a3aec79624c7daULL, 0x84547ddc3e203c94ULL,
    0x990a98fd5071d263ULL, 0x1a4ff12616eefc89ULL, 0xf6f7fd1431714200ULL,
    0x30c05b1ba332f41cULL, 0x8d2636b81555a786ULL, 0x46c9feb55d120902ULL,
    0xccec0a73b49c9921ULL, 0x4e9d2827355fc492ULL, 0x19ebb029435dcb0fULL,
    0x4659d2b743848a2cULL, 0x963ef2c96b33be31ULL, 0x74f85198b05a2e7dULL,
    0x5a0f544dd2b1fb18ULL, 0x03727073c2e134b1ULL, 0xc7f6aa2de59aea61ULL,
    0x352787baa0d7c22fULL, 0x9853eab63b5e0b35ULL, 0xabbdcdd7ed5c0860ULL,
    0xcf05daf5ac8d77b0ULL, 0x49cad48cebf4a71eULL, 0x7a4c10ec2158c4a6ULL,
    0xd9e92aa246bf719eULL, 0x13ae978d09fe5557ULL, 0x730499af921549ffULL,
    0x4e4b705b92903ba4ULL, 0xff577222c14f0a3aULL, 0x55b6344cf97aafaeULL,
    0xb862225b055b6960ULL, 0xcac09afbddd2cdb4ULL, 0xdaf8e9829fe96b5fULL,
    0xb5fdfc5d3132c498ULL, 0x310cb380db6f7503ULL, 0xe87fbb46217a360eULL,
    0x2102ae466ebb1148ULL, 0xf8549e1a3aa5e00dULL, 0x07a69afdcc42261aULL,
    0xc4c118bfe78feaaeULL, 0xf9f4892ed96bd438ULL, 0x1af3dbe25d8f45daULL,
    0xf5b4b0b0d2deeeb4ULL, 0x962aceefa82e1c84ULL, 0x046e3ecaaf453ce9ULL,
    0xf05d129681949a4cULL, 0x964781ce734b3c84ULL, 0x9c2ed44081ce5fbdULL,
    0x522e23f3925e319eULL, 0x177e00f9fc32f791ULL, 0x2bc60a63a6f3b3f2ULL,
    0x222bbfae61725606ULL, 0x486289ddcc3d6780ULL, 0x7dc7785b8efdfc80ULL,
    0x8af38731c02ba980ULL, 0x1fab64ea29a2ddf7ULL, 0xe4d9429322cd065aULL,
    0x9da058c67844f20cULL, 0x24c0e332b70019b0ULL, 0x233003b5a6cfe6adULL,
    0xd586bd01c5c217f6ULL, 0x5e5637885f29bc2bULL, 0x7eba726d8c94094bULL,
    0x0a56a5f0bfe39272ULL, 0xd79476a84ee20d06ULL, 0x9e4c1269baa4bf37ULL,
    0x17efee45b0dee640ULL, 0x1d95b0a5fcf90bc6ULL, 0x93cbe0b699c2585dULL,
    0x65fa4f227a2b6d79ULL, 0xd5f9e858292504d5ULL, 0xc2b5a03f71471a6fULL,
    0x59300222b4561e00ULL, 0xce2f8642ca0712dcULL, 0x7ca9723fbb2e8988ULL,
    0x2785338347f2ba08ULL, 0xc61bb3a141e50e8cULL, 0x150f361dab9dec26ULL,
    0x9f6a419d382595f4ULL, 0x64a53dc924fe7ac9ULL, 0x142de49fff7a7c3dULL,
    0x0c335248857fa9e7ULL, 0x0a9c32d5eae45305ULL, 0xe6c42178c4bbb92eULL,
    0x71f1ce2490d20b07ULL, 0xf1bcc3d275afe51aULL, 0xe728e8c83c334074ULL,
    0x96fbf83a12884624ULL, 0x81a1549fd6573da5ULL, 0x5fa7867caf35e149ULL,
    0x56986e2ef3ed091bULL, 0x917f1dd5f8886c61ULL, 0xd20d8c88c8ffe65fULL,
    0x31d71dce64b2c310ULL, 0xf165b587df898190ULL, 0xa57e6339dd2cf3a0ULL,
    0x1ef6e6dbb1961ec9ULL, 0x70cc73d90bc26e24ULL, 0xe21a6b35df0c3ad7ULL,
    0x003a93d8b2806962ULL, 0x1c99ded33cb890a1ULL, 0xcf3145de0add4289ULL,
    0xd0e4427a5514fb72ULL, 0x77c621cc9fb3a483ULL, 0x67a34dac4356550bULL,
    0xf8d626aaaf278509ULL
};
