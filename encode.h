#ifndef ENCODE_H_
#define ENCODE_H_

char *encode_uint(char *buffer, unsigned long value);
char *decode_uint(char *buffer, unsigned long *value);

#endif  // #ifndef ENCODE_H_
