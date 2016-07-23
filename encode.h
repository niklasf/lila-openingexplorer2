#ifndef ENCODE_H_
#define ENCODE_H_

#include <stdint.h>

#include "move.h"

uint8_t *encode_uint(uint8_t *buffer, unsigned long value);
const uint8_t *decode_uint(const uint8_t *buffer, unsigned long *value);

uint8_t *encode_uint48(uint8_t *buffer, uint64_t value);
const uint8_t *decode_uint48(const uint8_t *buffer, uint64_t *value);

uint8_t *encode_game_id(uint8_t *buffer, const char *game_id);
const uint8_t *decode_game_id(const uint8_t *buffer, char *game_id);

struct move_stats {
    move_t move;
    unsigned long white;
    unsigned long draws;
    unsigned long black;
    unsigned long average_rating_sum;
};

struct master_ref {
    char game_id[8];
    unsigned average_rating;
};

static const size_t MASTER_MAX_REFS = 4;

struct master_record {
    unsigned num_moves;
    unsigned num_refs;

    struct move_stats *moves;

    struct master_ref refs[MASTER_MAX_REFS];
};

struct master_record *master_record_new();

void master_record_add_move(struct master_record *record,
                            move_t move, const struct master_ref *ref, int wdl);

uint8_t *encode_master_record(uint8_t *buffer, const struct master_record *record);
const uint8_t *decode_master_record(const uint8_t *buffer, struct master_record *record);
void master_record_print(const struct master_record *record);
void master_record_free(struct master_record *record);

#endif  // #ifndef ENCODE_H_
