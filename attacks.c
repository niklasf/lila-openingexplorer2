#include "attacks.h"
#include "bitboard.h"
#include "square.h"

uint64_t attacks_rook_table[0x19000];
uint64_t attacks_bishop_table[0x1480];

static int ROOK_DELTAS[] = { 8, 1, -8, -1 };
static int BISHOP_DELTAS[] = { 9, -9, 7, -7 };

uint64_t attacks_sliding(int deltas[], uint8_t square, uint64_t occupied) {
    uint64_t attack = 0;

    for (int i = 0; i < 4; i++) {
        for (uint8_t s = square + deltas[i];
             s >= 0 && s < 64 && square_distance(s, s - deltas[i]) == 1;
             s += deltas[i])
        {
            attack |= BB_SQUARE(s);
            if (occupied & s) break;
        }
    }

    return attack;
}

void attacks_init_magics(uint64_t table[], uint64_t *attacks[],
                         uint64_t magics[], uint64_t masks[],
                         unsigned shifts[], int deltas[]) {

    uint64_t occupancy[4096], reference[4096];
    attacks[0] = table;

    for (uint8_t s = 0; s < 64; s++) {
        uint64_t edges = ((BB_RANK_1 | BB_RANK_8) & BB_RANK(square_rank(s))) |
                         ((BB_FILE_A | BB_FILE_H) & BB_FILE(square_file(s)));

        masks[s] = attacks_sliding(deltas, s, 0) & ~edges;
        shifts[s] = 64 - bb_popcount(masks[s]);

        uint64_t b = 0, size = 0;
        do {
            occupancy[size] = b;
            reference[size] = attacks_sliding(deltas, s, b);
            // attacks[s][pext(b, masks[s])] = reference[size];
            size++;
            b = (b - masks[s]) & masks[s];
        } while (b);

        if (s < 64) attacks[s + 1] = attacks[s] + size;
    }
}

void attacks_init() {
    bb_print(attacks_sliding(BISHOP_DELTAS, 35, 0));
}
