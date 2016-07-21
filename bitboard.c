#include <stdio.h>

#include "bitboard.h"
#include "square.h"

void bb_print(uint64_t bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            if (BB_SQUARE(square(file, rank)) & bb) printf("1");
            else printf(".");

            if (file == 7) printf("\n");
            else printf(" ");
        }
    }
}
