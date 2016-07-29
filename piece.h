#ifndef PIECE_H_
#define PIECE_H_

typedef enum {
    kAll, kPawn, kKnight, kBishop, kRook, kQueen, kKing,
    kNone = 0
} piece_type_t;

typedef enum {
    kBlack, kWhite
} color_t;

static const char PIECE_SYMBOLS[] = "\0pnbrqk";

static inline char piece_symbol(piece_type_t piece_type, bool color) {
    char symbol = PIECE_SYMBOLS[piece_type];
    return (color && symbol) ? (symbol + ('A' - 'a')) : symbol;
}

static inline piece_type_t piece_type_from_symbol(char symbol) {
    switch (symbol) {
        case 'p':
        case 'P':
            return kPawn;
        case 'n':
        case 'N':
            return kKnight;
        case 'b':
        case 'B':
            return kBishop;
        case 'r':
        case 'R':
            return kRook;
        case 'q':
        case 'Q':
            return kQueen;
        case 'k':
        case 'K':
            return kKing;
        default:
            return kNone;
    }
}

#endif  // #ifndef PIECE_H_
