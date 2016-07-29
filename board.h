#ifndef BOARD_H_
#define BOARD_H_

#include <stdbool.h>
#include <stdint.h>

#include "move.h"
#include "piece.h"

typedef struct board {
    uint64_t occupied_co[2];
    uint64_t occupied[7];

    bool turn;

    uint64_t castling;
    uint8_t ep_square;

    int halfmove_clock;
    int fullmove_number;
} board_t;

void board_clear(struct board *pos);
void board_reset(struct board *pos);
uint64_t board_pieces(const board_t *pos, piece_type_t pt, color_t color);
void board_set_piece_at(board_t *pos, square_t square, piece_type_t pt, color_t color);

bool board_set_fen(struct board *pos, const char *fen);
char *board_shredder_fen(const struct board *pos, char *fen);

void board_print(const struct board *pos);

bool board_is_insufficient_material(const struct board *pos);

void board_remove_piece_at(struct board *pos, square_t square);

uint64_t board_castling_rights(const struct board *pos);
uint64_t board_attacks_to(const struct board *pos, uint8_t square);
uint64_t board_attacks_from(const struct board *pos, uint8_t square);
uint64_t board_checkers(const struct board *pos, bool turn);
void board_move(board_t *pos, move_t move);
move_t *board_pseudo_legal_moves(const struct board *pos, move_t *moves, uint64_t from_mask, uint64_t to_mask);
move_t *board_legal_moves(const struct board *pos, move_t *moves, uint64_t from_mask, uint64_t to_mask);
uint64_t board_zobrist_hash(const struct board *pos, const uint64_t array[]);
bool board_parse_san(const struct board *pos, const char *san, move_t *move);

static const size_t LEN_SAN = 8;
char *board_san(const struct board *pos, move_t move, char *san);

extern const uint64_t POLYGLOT[];

#endif  // #ifndef BOARD_H_
