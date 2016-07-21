#include <stdio.h>
#include <assert.h>

#include "attacks.h"
#include "bitboard.h"

void test_attacks_rook() {
    puts("test_attacks_rook");
    assert(bb_popcount(attacks_rook(34, BB_VOID)) == 14);
}

int main() {
    attacks_init();

    test_attacks_rook();
    return 0;
}
