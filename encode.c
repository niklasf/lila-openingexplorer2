#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "encode.h"

uint8_t *encode_uint(uint8_t *buffer, unsigned long value) {
    while (value > 127) {
        *buffer++ = (value & 127) | 128;
        value >>= 7;
    }

    *buffer++ = value & 127;

    return buffer;
}

const uint8_t *decode_uint(const uint8_t *buffer, unsigned long *value) {
    *value = 0;

    size_t i = 0;

    do {
        *value |= (*buffer & 127) << (7 * i++);
    } while (*buffer++ & 128);

    return buffer;
}

uint8_t *encode_uint48(uint8_t *buffer, uint64_t value) {
    *buffer++ = value & 255;
    *buffer++ = (value >> 8) & 255;
    *buffer++ = (value >> 16) & 255;
    *buffer++ = (value >> 24) & 255;
    *buffer++ = (value >> 32) & 255;
    *buffer++ = (value >> 40) & 255;
    return buffer;
}

const uint8_t *decode_uint48(const uint8_t *buffer, uint64_t *value) {
    *value = 0;
    *value |= ((uint64_t) *buffer++);
    *value |= ((uint64_t) *buffer++) << 8;
    *value |= ((uint64_t) *buffer++) << 16;
    *value |= ((uint64_t) *buffer++) << 24;
    *value |= ((uint64_t) *buffer++) << 32;
    *value |= ((uint64_t) *buffer++) << 40;
    return buffer;
}

static const char BASE_62[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

uint8_t *encode_game_id(uint8_t *buffer, const char *game_id) {
    uint64_t bytes = 0;

    for (int i = 0; i < 8; i++) {
        bytes = bytes * 62;
        if (game_id[i] >= '0' && game_id[i] <= '9') bytes += game_id[i] - '0';
        else if (game_id[i] >= 'A' && game_id[i] <= 'Z') bytes += game_id[i] - 'A' + 10;
        else if (game_id[i] >= 'a' && game_id[i] <= 'z') bytes += game_id[i] - 'a' + 10 + 26;
        else assert(false);
    }

    return encode_uint48(buffer, bytes);
}

const uint8_t *decode_game_id(const uint8_t *buffer, char *game_id) {
    uint64_t bytes;
    buffer = decode_uint48(buffer, &bytes);

    for (int i = 7; i >= 0; i--) {
        lldiv_t r = lldiv(bytes, 62);
        game_id[i] = BASE_62[r.rem];
        bytes = r.quot;
    }

    game_id[8] = 0;

    return buffer;
}

struct master_record *master_record_new() {
    struct master_record *record = malloc(sizeof(struct master_record));
    if (!record) return NULL;

    record->num_refs = 0;
    record->num_moves = 0;
    record->moves = NULL;
    record->refs = NULL;
    return record;
}

void master_record_free(struct master_record *record) {
    assert(record);
    if (record->moves) free(record->moves);
    if (record->refs) free(record->refs);
    free(record);
}

uint8_t *master_record_encode(const struct master_record *record, uint8_t *buffer) {
    buffer = encode_uint(buffer, record->num_refs);
    buffer = encode_uint(buffer, record->num_moves);
    return buffer;
}
