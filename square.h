#ifndef SQUARE_H_
#define SQUARE_H_

#include <stdint.h>

typedef uint8_t square_t;

static inline uint8_t square(int file, int rank) {
    return file + 8 * rank;
}

static inline int square_file(uint8_t square) {
    return square & 7;
}

static inline int square_rank(uint8_t square) {
    return square >> 3;
}

char *square_name(uint8_t square, char *name);

int square_distance(uint8_t a, uint8_t b);

#define SQ_A1 0
#define SQ_B1 1
#define SQ_C1 2
#define SQ_D1 3
#define SQ_E1 4
#define SQ_F1 5
#define SQ_G1 6
#define SQ_H1 7
#define SQ_A2 8
#define SQ_B2 9
#define SQ_C2 10
#define SQ_D2 11
#define SQ_E2 12
#define SQ_F2 13
#define SQ_G2 14
#define SQ_H2 15
#define SQ_A3 16
#define SQ_B3 17
#define SQ_C3 18
#define SQ_D3 19
#define SQ_E3 20
#define SQ_F3 21
#define SQ_G3 22
#define SQ_H3 23
#define SQ_A4 24
#define SQ_B4 25
#define SQ_C4 26
#define SQ_D4 27
#define SQ_E4 28
#define SQ_F4 29
#define SQ_G4 30
#define SQ_H4 31
#define SQ_A5 32
#define SQ_B5 33
#define SQ_C5 34
#define SQ_D5 35
#define SQ_E5 36
#define SQ_F5 37
#define SQ_G5 38
#define SQ_H5 39
#define SQ_A6 40
#define SQ_B6 41
#define SQ_C6 42
#define SQ_D6 43
#define SQ_E6 44
#define SQ_F6 45
#define SQ_G6 46
#define SQ_H6 47
#define SQ_A7 48
#define SQ_B7 49
#define SQ_C7 50
#define SQ_D7 51
#define SQ_E7 52
#define SQ_F7 53
#define SQ_G7 54
#define SQ_H7 55
#define SQ_A8 56
#define SQ_B8 57
#define SQ_C8 58
#define SQ_D8 59
#define SQ_E8 60
#define SQ_F8 61
#define SQ_G8 62
#define SQ_H8 63

#endif  // #ifndef SQUARE_H_
