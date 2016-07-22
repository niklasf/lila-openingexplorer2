#ifndef ENCODE_H_
#define ENCODE_H_

#include <stdint.h>

uint8_t *encode_uint(uint8_t *buffer, unsigned long value);
const uint8_t *decode_uint(const uint8_t *buffer, unsigned long *value);

uint8_t *encode_gameid(uint8_t *buffer, const char *gameid);
const uint8_t *decode_gameid(const uint8_t *buffer, char *gameid);


#endif  // #ifndef ENCODE_H_
