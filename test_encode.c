#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "encode.h"

void test_encode_uint() {
    puts("test_encode_uint");

    unsigned char buffer[16];
    unsigned char *end, *decode_end = buffer;

    for (unsigned long in = 0; in < 3000000ULL; in++) {
        end = encode_uint(buffer, in);
        if (end > decode_end) printf("- %lu\n", in);

        unsigned long out = 0;
        decode_end = (unsigned char *) decode_uint(buffer, &out);

        if (in != out) printf("%lu != %lu\n", in, out);
        assert(in == out);
        assert(end == decode_end);
    }
}

void test_encode_game_id() {
    puts("test_encode_game_id");
    unsigned char encoded[16] = { };
    char decoded[16] = { };

    const char *id_1 = "00000000";
    encode_game_id(encoded, id_1);
    decode_game_id(encoded, decoded);
    assert(strlen(decoded) > 0);
    printf("- %s %s\n", id_1, decoded);
    assert(strcmp(id_1, decoded) == 0);

    const char *id_2 = "000ZABE7";
    encode_game_id(encoded, id_2);
    decode_game_id(encoded, decoded);
    printf("- %s %s\n", id_2, decoded);
    assert(strcmp(id_2, decoded) == 0);

    const char *id_3 = "BE73q6WU";
    encode_game_id(encoded, id_3);
    decode_game_id(encoded, decoded);
    printf("- %s %s\n", id_3, decoded);
    assert(strcmp(id_3, decoded) == 0);

    const char *id_4 = "aaaaaaaa";
    encode_game_id(encoded, id_4);
    decode_game_id(encoded, decoded);
    printf("- %s %s\n", id_4, decoded);
    assert(strcmp(id_4, decoded) == 0);

    const char *id_5 = "AAAAAAAA";
    encode_game_id(encoded, id_5);
    decode_game_id(encoded, decoded);
    printf("- %s %s\n", id_5, decoded);
    assert(strcmp(id_5, decoded) == 0);

    const char *id_6 = "00zAAj00";
    encode_game_id(encoded, id_6);
    decode_game_id(encoded, decoded);
    printf("- %s %s\n", id_6, decoded);
    assert(strcmp(id_6, decoded) == 0);
}

void test_master_record() {
    puts("test_master_record");

    uint8_t buffer[255] = {};

    struct master_record *record = master_record_new();
    record->num_refs = 10;
    record->num_moves = 500;
    master_record_encode(record, buffer);
    master_record_free(record);

    for (int i = 0; i < 255; i++) printf("%d ", buffer[i]);
    puts("");
}

int main() {
    test_encode_uint();
    test_encode_game_id();
    test_master_record();
    return 0;
}
