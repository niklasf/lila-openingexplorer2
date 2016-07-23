#ifndef ENCODE_H_
#define ENCODE_H_

#include <stdint.h>

uint8_t *encode_uint(uint8_t *buffer, unsigned long value);
const uint8_t *decode_uint(const uint8_t *buffer, unsigned long *value);

uint8_t *encode_uint48(uint8_t *buffer, uint64_t value);
const uint8_t *decode_uint48(const uint8_t *buffer, uint64_t *value);

uint8_t *encode_game_id(uint8_t *buffer, const char *game_id);
const uint8_t *decode_game_id(const uint8_t *buffer, char *game_id);

#endif  // #ifndef ENCODE_H_
