#include <stdio.h>

#include "attacks.h"
#include "bitboard.h"
#include "square.h"

const static int ROOK_DELTAS[] = { 8, 1, -8, -1, 0 };
static uint64_t ROOK_TABLE[0x19000];
static uint64_t ROOK_MASKS[64];
static uint64_t ROOK_MAGICS[64];
static unsigned ROOK_SHIFTS[64];
static uint64_t *ROOK_ATTACKS[64];

const static int BISHOP_DELTAS[] = { 9, -9, 7, -7, 0 };
static uint64_t BISHOP_TABLE[0x1480];
static uint64_t BISHOP_MASKS[64];
static uint64_t BISHOP_MAGICS[64];
static unsigned BISHOP_SHIFTS[64];
static uint64_t *BISHOP_ATTACKS[64];

const static int KING_DELTAS[] = { 8, 1, -8, -1, 9, -9, 7, -7, 0 };
static uint64_t KING_ATTACKS[64];

const static int KNIGHT_DELTAS[] = { 17, 15, 10, 6, -6, -10, -15, -17, 0 };
static uint64_t KNIGHT_ATTACKS[64];

static uint64_t attacks_sliding(const int deltas[], uint8_t square, uint64_t occupied) {
    uint64_t attack = 0;

    for (int i = 0; deltas[i]; i++) {
        for (int s = square + deltas[i];
             s >= 0 && s < 64 && square_distance(s, s - deltas[i]) <= 2;
             s += deltas[i])
        {
            attack |= BB_SQUARE(s);
            if (occupied & attack) break;
        }
    }

    return attack;
}

static void attacks_init_magics(uint64_t table[], uint64_t *attacks[],
                         uint64_t magics[], uint64_t masks[],
                         unsigned shifts[], const int deltas[]) {

    uint64_t occupancy[4096], reference[4096];
    attacks[0] = table;

    for (uint8_t s = 0; s < 64; s++) {
        uint64_t edges = ((BB_RANK_1 | BB_RANK_8) & ~BB_RANK(square_rank(s))) |
                         ((BB_FILE_A | BB_FILE_H) & ~BB_FILE(square_file(s)));

        masks[s] = attacks_sliding(deltas, s, 0) & ~edges;
        shifts[s] = 64 - bb_popcount(masks[s]);

        uint64_t b = 0, size = 0;
        do {
            occupancy[size] = b;
            reference[size] = attacks_sliding(deltas, s, b);
            attacks[s][bb_pext(b, masks[s])] = reference[size];
            size++;
            b = (b - masks[s]) & masks[s];
        } while (b);

        if (s < 64) attacks[s + 1] = attacks[s] + size;
    }
}

uint64_t attacks_rook(uint8_t square, uint64_t occupied) {
    return ROOK_ATTACKS[square][bb_pext(occupied, ROOK_MASKS[square])];
}

uint64_t attacks_bishop(uint8_t square, uint64_t occupied) {
    return BISHOP_ATTACKS[square][bb_pext(occupied, BISHOP_MASKS[square])];
}

uint64_t attacks_knight(uint8_t square) {
    return KNIGHT_ATTACKS[square];
}

uint64_t attacks_king(uint8_t square) {
    return KING_ATTACKS[square];
}

void attacks_init() {
    attacks_init_magics(ROOK_TABLE, ROOK_ATTACKS,
                        ROOK_MAGICS, ROOK_MASKS, ROOK_SHIFTS,
                        ROOK_DELTAS);

    attacks_init_magics(BISHOP_TABLE, BISHOP_ATTACKS,
                        BISHOP_MAGICS, BISHOP_MASKS, BISHOP_SHIFTS,
                        BISHOP_DELTAS);

    for (uint8_t s = 0; s < 64; s++) {
        KNIGHT_ATTACKS[s] = attacks_sliding(KNIGHT_DELTAS, s, BB_ALL);
        KING_ATTACKS[s] = attacks_sliding(KING_DELTAS, s, BB_ALL);
    }
}
