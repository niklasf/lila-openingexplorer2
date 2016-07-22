#ifndef ENCODE_H_
#define ENCODE_H_

char *encode_uint(char *buffer, unsigned long value);
const char *decode_uint(const char *buffer, unsigned long *value);

char *encode_gameid(char *buffer, const char *gameid);
const char *decode_gameid(const char *buffer, char *gameid);


#endif  // #ifndef ENCODE_H_
