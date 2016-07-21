#include <assert.h>
#include <stdio.h>

#include "bitboard.h"

void test_bb_lsb() {
    puts("test_bb_lsb");

    assert(bb_lsb(3) == 0);
    assert(bb_lsb(8) == 3);
}

void test_bb_msb() {
    puts("test_bb_msb");

    assert(bb_msb(8) == 3);
    assert(bb_msb(BB_ALL) == 63);
}

void test_bb_popmsb() {
    puts("test_bb_popmsb");

    uint64_t bb = BB_ALL;
    uint8_t msb = bb_popmsb(&bb);
    assert(msb == 63);
    assert(bb == (BB_ALL ^ BB_H8));
}

void test_bb_popcount() {
    puts("test_bb_popcount");

    assert(bb_popcount(BB_A1 | BB_A2 | BB_H7) == 3);
    assert(bb_popcount(BB_VOID) == 0);
    assert(bb_popcount(BB_ALL) == 64);
}

int main() {
    test_bb_lsb();
    test_bb_msb();
    test_bb_popmsb();
    test_bb_popcount();
}
