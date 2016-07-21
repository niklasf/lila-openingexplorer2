#include "square.h"

char *square_name(uint8_t square, char *name) {
    *name++ = 'a' + square_file(square);
    *name++ = '1' + square_rank(square);
    return name;
}
