#include <stdio.h>
#include <assert.h>

#include "attacks.h"
#include "bitboard.h"
#include "square.h"

void test_attacks_rook() {
    puts("test_attacks_rook");
    assert(bb_popcount(attacks_rook(SQ_C7, BB_VOID)) == 14);
    assert(bb_popcount(attacks_rook(SQ_H1, BB_VOID)) == 14);

    uint64_t attacks = attacks_rook(SQ_H1, BB_H2 | BB_E1);
    bb_print(attacks);
    assert(bb_popcount(attacks) == 4);
}

void test_attacks_knight() {
    puts("test_attacks_knight");
    bb_print(attacks_knight(SQ_D4));
    assert(bb_popcount(attacks_knight(SQ_D4)) == 8);
}

void test_attacks_bishop() {
    puts("test_attacks_bishop");

    uint64_t attacks = attacks_bishop(SQ_F1, BB_G1 | BB_F2);
    bb_print(attacks);
    assert(!(BB_F2 & attacks));
}

int main() {
    attacks_init();

    test_attacks_rook();
    test_attacks_knight();
    test_attacks_bishop();
    return 0;
}
