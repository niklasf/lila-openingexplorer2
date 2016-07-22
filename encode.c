#include <stdlib.h>

#include "encode.h"

char *encode_uint(char *buffer, unsigned long value) {
    while (value > 127) {
        *buffer++ = (value & 127) | 128;
        value >>= 7;
    }

    *buffer++ = value & 127;

    return buffer;
}

char *decode_uint(char *buffer, unsigned long *value) {
    *value = 0;

    size_t i = 0;

    do {
        *value |= (*buffer & 127) << (7 * i++);
    } while (*buffer++ & 128);

    return buffer;
}
