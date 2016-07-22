#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

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

static const char BASE_62[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char *encode_gameid(char *buffer, const char *game_id) {
    uint64_t bytes = 0;

    for (int i = 0; i < 12; i++) {
        bytes = bytes * 62;
        if (game_id[i] >= '0' && game_id[i] <= '9') bytes += game_id[i] - '0';
        else if (game_id[i] >= 'A' && game_id[i] <= 'Z') bytes += game_id[i] - 'A' + 10;
        else if (game_id[i] >= 'a' && game_id[i] <= 'z') bytes += game_id[i] - 'a' + 10 + 26;
        else assert(false);
    }

    printf("o: %lu\n", bytes);

    *buffer++ = bytes & 255;
    *buffer++ = (bytes >> 8) & 255;
    *buffer++ = (bytes >> 16) & 255;
    *buffer++ = (bytes >> 24) & 255;
    *buffer++ = (bytes >> 32) & 255;
    *buffer++ = (bytes >> 40) & 255;

    return buffer;
}

const char *decode_gameid(const char *buffer, char *game_id) {
    uint64_t bytes = 0;
    bytes |= *buffer++;
    bytes |= *buffer++ << 8;
    bytes |= *buffer++ << 16;
    bytes |= *buffer++ << 24;
    bytes |= *buffer++ << 32;
    bytes |= *buffer++ << 40;

    printf("i: %lu\n", bytes);

    for (int i = 11; i >= 0; i--) {
        lldiv_t r = lldiv(bytes, 62);
        game_id[i] = BASE_62[r.rem];
        bytes = r.quot;
    }

    game_id[12] = 0;

    return buffer;
}
