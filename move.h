#ifndef MOVE_H_
#define MOVE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "piece.h"
#include "square.h"

// from:   1 -  6
// to:     7 - 12
// piece: 12 - 16
typedef uint16_t move_t;

static inline move_t move_make(square_t from, square_t to, piece_type_t pt) {
    return from | to << 6 | pt << 12;
}

static inline move_t move_drop(square_t square, piece_type_t pt) {
    assert(pt != kNone);
    return move_make(square, square, pt);
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
    return (move >> 12) & 7;
}

static const size_t LEN_UCI = 6;

char *move_uci(move_t move, char *uci);

#endif  // #ifndef MOVE_H_
