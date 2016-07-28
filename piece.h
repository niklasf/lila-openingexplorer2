#ifndef PIECE_H_
#define PIECE_H_

typedef enum {
  kPawn,
  kKnight,
  kBishop,
  kRook,
  kQueen,
  kKing,
  kNone
} piece_type_t;

static const char PIECE_SYMBOLS[] = "\0pnbrqk";

static inline char piece_symbol(piece_type_t piece_type, bool color) {
    char symbol = PIECE_SYMBOLS[piece_type];
    return (color && symbol) ? (symbol + ('A' - 'a')) : symbol;
}

#endif  // #ifndef PIECE_H_
