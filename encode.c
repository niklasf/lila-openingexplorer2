#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "move.h"

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

uint8_t *encode_uint16(uint8_t *buffer, uint16_t value) {
    *((uint16_t *) buffer) = value;
    return buffer + 2;
}

const uint8_t *decode_uint16(const uint8_t *buffer, uint16_t *value) {
    *value = *((uint16_t *) buffer);
    return buffer + 2;
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
    if (!record) abort();

    record->num_refs = 0;
    record->num_moves = 0;
    record->moves = NULL;
    return record;
}

void master_record_free(struct master_record *record) {
    assert(record);
    if (record->moves) free(record->moves);
    free(record);
}

void master_record_add_move(struct master_record *record,
                            move_t move, const struct master_ref *ref, int wdl) {

    if (record->num_refs < MASTER_MAX_REFS) {
        record->refs[record->num_refs++] = *ref;
    } else {
        // TODO: Replace lowest game.
    }

    for (size_t i = 0; i < record->num_moves; i++) {
        if (record->moves[i].move == move) {
            if (wdl > 0) record->moves[i].white++;
            else if (wdl == 0) record->moves[i].draws++;
            else record->moves[i].black++;

            record->moves[i].average_rating_sum += ref->average_rating;
            return;
        }
    }

    struct move_stats *stats =
        realloc(record->moves, sizeof(struct move_stats) * (record->num_moves + 1));
    if (!stats) abort();

    stats[record->num_moves].move = move;
    stats[record->num_moves].white = (wdl > 0) ? 1 : 0;
    stats[record->num_moves].draws = (wdl == 0) ? 1 : 0;
    stats[record->num_moves].black = (wdl < 0) ? 1 : 0;
    stats[record->num_moves].average_rating_sum = ref->average_rating;

    record->num_moves++;
    record->moves = stats;
}

uint8_t *encode_master_record(uint8_t *buffer, const struct master_record *record) {
    buffer = encode_uint(buffer, record->num_refs);
    buffer = encode_uint(buffer, record->num_moves);

    for (size_t i = 0; i < record->num_moves; i++) {
        buffer = encode_uint16(buffer, record->moves[i].move);
        buffer = encode_uint(buffer, record->moves[i].white);
        buffer = encode_uint(buffer, record->moves[i].draws);
        buffer = encode_uint(buffer, record->moves[i].black);
        buffer = encode_uint(buffer, record->moves[i].average_rating_sum);
    }

    for (size_t i = 0; i < record->num_refs; i++) {
        buffer = encode_game_id(buffer, record->refs[i].game_id);
        buffer = encode_uint(buffer, record->refs[i].average_rating);
    }

    return buffer;
}

const uint8_t *decode_master_record(const uint8_t *buffer, struct master_record *record) {
    unsigned long num_refs;
    buffer = decode_uint(buffer, &num_refs);
    record->num_refs = num_refs;

    unsigned long num_moves;
    buffer = decode_uint(buffer, &num_moves);
    record->num_moves = num_moves;

    record->moves = realloc(record->moves, sizeof(struct move_stats) * record->num_moves);
    if (!record->moves) abort();

    for (size_t i = 0; i < record->num_moves; i++) {
        buffer = decode_uint16(buffer, &record->moves[i].move);
        buffer = decode_uint(buffer, &record->moves[i].white);
        buffer = decode_uint(buffer, &record->moves[i].draws);
        buffer = decode_uint(buffer, &record->moves[i].black);
        buffer = decode_uint(buffer, &record->moves[i].average_rating_sum);
    }

    for (size_t i = 0; i < record->num_refs; i++) {
        buffer = decode_game_id(buffer, record->refs[i].game_id);

        unsigned long average_rating;
        buffer = decode_uint(buffer, &average_rating);
        record->refs[i].average_rating = average_rating;
    }

    return buffer;
}

void master_record_print(const struct master_record *record) {
    printf("num_moves: %d\n", record->num_moves);

    for (size_t i = 0; i < record->num_moves; i++) {
        unsigned long total = record->moves[i].white + record->moves[i].draws + record->moves[i].black;

        char uci[LEN_UCI];
        move_uci(record->moves[i].move, uci);
        printf("  %s (t: %lu, w: %lu, d: %lu, b: %lu, avg: %lu)\n", uci,
            total,
            record->moves[i].white,
            record->moves[i].draws,
            record->moves[i].black,
            record->moves[i].average_rating_sum / total);
    }

    printf("num_refs: %d\n", record->num_refs);

    for (size_t i = 0; i < record->num_refs; i++) {
        char c_game_id[9];
        strncpy(c_game_id, record->refs[i].game_id, 8);
        printf("  %s (avg: %d)\n", c_game_id, record->refs[i].average_rating);
    }
}
