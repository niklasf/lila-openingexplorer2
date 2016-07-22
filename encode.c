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

const char *decode_uint(const char *buffer, unsigned long *value) {
    *value = 0;

    size_t i = 0;

    do {
        *value |= (*buffer & 127) << (7 * i++);
    } while (*buffer++ & 128);

    return buffer;
}

char *encode_gameid(char *buffer, const char *game_id) {
    return buffer;
}

const char *decode_gameid(const char *buffer, char *game_id) {
    return buffer;
}
