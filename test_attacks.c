#include <stdio.h>
#include <assert.h>

#include "attacks.h"
#include "bitboard.h"
#include "square.h"

void test_attacks_rook() {
    puts("test_attacks_rook");
    assert(bb_popcount(attacks_rook(SQ_C7, BB_VOID)) == 14);
}

void test_attacks_knight() {
    puts("test_attacks_knight");
    bb_print(attacks_knight(SQ_D4));
    assert(bb_popcount(attacks_knight(SQ_D4)) == 8);
}

int main() {
    attacks_init();

    test_attacks_rook();
    test_attacks_knight();
    return 0;
}
