#ifndef BOARD_H_
#define BOARD_H_

#include <stdbool.h>
#include <stdint.h>

/* Boards */

struct board {
    uint64_t white;
    uint64_t black;
    uint64_t kings;
    uint64_t queens;
    uint64_t rooks;
    uint64_t bishops;
    uint64_t knights;
    uint64_t pawns;

    bool turn;

    uint64_t castling;

    uint8_t ep_square;

    int halfmove_clock;
    int fullmove_number;
};

void board_print(const struct board *pos);
void board_clear(struct board *pos);
void board_reset(struct board *pos);
char *board_shredder_fen(const struct board *pos, char *fen);
bool board_set_fen(struct board *pos, const char *fen);

#endif  // #ifndef BOARD_H_
