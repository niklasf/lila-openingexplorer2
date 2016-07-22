#ifndef MOVE_H_
#define MOVE_H_

#include <stdint.h>
#include <stdbool.h>

#include "square.h"

// from:   1 -  6
// to:     7 - 12
// piece: 12 - 16
typedef uint16_t move_t;

static inline move_t move_make(square_t from, square_t to, char piece_type) {
    move_t move = from | to << 6;

    if (piece_type) {
        if (piece_type == 'p') move |= 1 << 12;
        else if (piece_type == 'n') move |= 2 << 12;
        else if (piece_type == 'b') move |= 3 << 12;
        else if (piece_type == 'r') move |= 4 << 12;
        else if (piece_type == 'q') move |= 5 << 12;
        else if (piece_type == 'k') move |= 6 << 12;
        else assert(false);
    }

    return move;
}

static inline move_t move_drop(square_t square, char piece_type) {
    assert(piece_type);
    return move_make(square, square, piece_type);
}

static inline square_t move_from(move_t move) {
    return move & 63;
}

static inline square_t move_to(move_t move) {
    return (move >> 6) & 63;
}

static inline bool move_is_drop(move_t move) {
    return move && move_from(move) == move_to(move);
}

static inline char move_piece_type(move_t move) {
    if (move & 7 << 12) {
        if (move & 1 << 12) return 'p';
        else if (move & 2 << 12) return 'n';
        else if (move & 3 << 12) return 'b';
        else if (move & 4 << 12) return 'r';
        else if (move & 5 << 12) return 'q';
        else if (move & 6 << 12) return 'k';
    }

    return 0;
}

#endif  // #ifndef MOVE_H_
