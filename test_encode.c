#include <stdio.h>
#include <assert.h>

#include "encode.h"

void test_encode_uint() {
    puts("test_encode_uint");

    char buffer[16];
    char *end, *decode_end = buffer;

    for (unsigned long in = 0; in < 3000000ULL; in++) {
        end = encode_uint(buffer, in);
        if (end > decode_end) printf("%lu\n", in);

        unsigned long out = 0;
        decode_end = decode_uint(buffer, &out);

        if (in != out) printf("%lu != %lu\n", in, out);
        assert(in == out);
        assert(end == decode_end);
    }
}

int main() {
    test_encode_uint();
    return 0;
}
