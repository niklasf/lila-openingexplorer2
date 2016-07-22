#include <stdio.h>
#include <assert.h>

#include "encode.h"

void test_encode_uint() {
    puts("test_encode_uint");

    char buffer[16];
    char *end, *decode_end = buffer;

    for (unsigned long in = 0; in < 3000000ULL; in++) {
        end = encode_uint(buffer, in);
        if (end > decode_end) printf("- %lu\n", in);

        unsigned long out = 0;
        decode_end = decode_uint(buffer, &out);

        if (in != out) printf("%lu != %lu\n", in, out);
        assert(in == out);
        assert(end == decode_end);
    }
}

void test_encode_gameid() {
    puts("test_encode_gameid");
    char encoded[16] = { };
    char decoded[16] = { };

    const char *id_1 = "000000000000";
    encode_gameid(encoded, id_1);
    decode_gameid(encoded, decoded);
    assert(strlen(decoded) > 0);
    printf("- %s %s\n", id_1, decoded);
    assert(strcmp(id_1, decoded) == 0);

    const char *id_2 = "000000000010";
    encode_gameid(encoded, id_2);
    decode_gameid(encoded, decoded);
    printf("- %s %s\n", id_2, decoded);
    assert(strcmp(id_2, decoded) == 0);

    const char *id_3 = "BE73q6WUCBeu";
    encode_gameid(encoded, id_3);
    decode_gameid(encoded, decoded);
    printf("- %s %s\n", id_3, decoded);
    assert(strcmp(id_3, decoded) == 0);

    const char *id_4 = "aaaaaaaaaaaa";
    encode_gameid(encoded, id_4);
    decode_gameid(encoded, decoded);
    printf("- %s %s\n", id_4, decoded);
    assert(strcmp(id_4, decoded) == 0);

    const char *id_5 = "AAAAAAAAAAAA";
    encode_gameid(encoded, id_5);
    decode_gameid(encoded, decoded);
    printf("- %s %s\n", id_5, decoded);
    assert(strcmp(id_5, decoded) == 0);
}

int main() {
    test_encode_uint();
    test_encode_gameid();
    return 0;
}
