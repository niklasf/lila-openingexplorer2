#ifndef SQUARE_H_
#define SQUARE_H_

#include <stdint.h>

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

#endif  // #ifndef SQUARE_H_
