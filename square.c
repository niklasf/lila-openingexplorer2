#include <stdlib.h>

#include "square.h"

char *square_name(uint8_t square, char *name) {
    *name++ = 'a' + square_file(square);
    *name++ = '1' + square_rank(square);
    *name = '\0';
    return name;
}

int square_distance(uint8_t a, uint8_t b) {
    int rd = abs(square_rank(a) - square_rank(b));
    int fd = abs(square_file(a) - square_file(b));
    if (rd > fd) return rd;
    else return fd;
}
