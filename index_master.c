#include <stdbool.h>

#include <kclangc.h>

#include "attacks.h"
#include "board.h"
#include "encode.h"

static char master_entry_buffer[8000] = {};

static KCDB *master_db;

struct master_delta {
    move_t move;
    struct master_ref ref;
    int result;
};

const char *merge_master_full(const char *hash, size_t hash_size,
                              const char *buf, size_t buf_size,
                              size_t *sp, void *opq) {

    const struct master_delta *delta = (struct master_delta *) opq;

    struct master_record *record = master_record_new();
    decode_master_record((const uint8_t *) buf, record);
    master_record_add_move(record, delta->move, &delta->ref, delta->result);
    // master_record_print(record);

    char *end = (char *) encode_master_record((uint8_t *) master_entry_buffer, record);
    *sp = end - master_entry_buffer;
    master_record_free(record);

    return master_entry_buffer;
}

const char *merge_master_empty(const char *hash, size_t hash_size,
                               size_t *sp, void *opq) {

    const struct master_delta *delta = (struct master_delta *) opq;

    struct master_record *record = master_record_new();
    master_record_add_move(record, delta->move, &delta->ref, delta->result);

    char *end = (char *) encode_master_record((uint8_t *) master_entry_buffer, record);
    *sp = end - master_entry_buffer;
    master_record_free(record);

    return master_entry_buffer;
}


const char *visit_master_pgn(const char *game_id, size_t game_id_size,
                             const char *buf, size_t buf_size,
                             size_t *sp, void *opq) {
    char *pgn = strndup(buf, buf_size);
    char *saveptr_pgn, *saveptr_line;

    int white_elo = 0, black_elo = 0;
    int result = 0;

    char *line = strtok_r(pgn, "\n", &saveptr_pgn);

    // Parse headers.
    for (; line && line[0] == '['; line = strtok_r(NULL, "\n", &saveptr_pgn)) {
        if (0 == strcmp(line, "[Result \"1/2-1/2\"]")) result = 0;
        else if (0 == strcmp(line, "[Result \"1-0\"]")) result = 1;
        else if (0 == strcmp(line, "[Result \"0-1\"]")) result = -1;
        else if (1 == sscanf(line, "[WhiteElo \"%d\"]", &white_elo)) continue;
        else if (1 == sscanf(line, "[BlackElo \"%d\"]", &black_elo)) continue;
    }

    assert(white_elo > 0);
    assert(black_elo > 0);

    board_t pos;
    board_reset(&pos);

    // Parse movetext.
    while (line) {
        char *token = strtok_r(line, " ", &saveptr_line);

        for (; token; token = strtok_r(NULL, " ", &saveptr_line)) {
            // Skip move numbers.
            if ('1' <= token[0] && token[0] <= '9') continue;

            // Parse moves.
            move_t move;
            if (board_parse_san(&pos, token, &move)) {
                uint64_t zobrist_hash = board_zobrist_hash(&pos, POLYGLOT);

                struct master_delta delta;
                delta.move = move;
                strncpy(delta.ref.game_id, game_id, 8);
                delta.ref.average_rating = (white_elo + black_elo) / 2;
                delta.result = result;

                if (!kcdbaccept(master_db, (char *) &zobrist_hash, sizeof(uint64_t),
                                merge_master_full, merge_master_empty,
                                &delta, true)) {
                    printf("master.kch accept error: %s\n", kcecodename(kcdbecode(master_db)));
                    abort();
                }

                board_move(&pos, move);
            } else {
                printf("illegal token: %s\n", token);
            }
        }

        line = strtok_r(NULL, "\n", &saveptr_pgn);
    }

    free(pgn);
    return KCVISNOP;
}

int main() {
    attacks_init();

    KCDB *master_pgn_db = kcdbnew();
    if (!kcdbopen(master_pgn_db, "master-pgn.kct", KCOREADER)) {
        printf("master-pgn.kct open error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
        return 1;
    }

    master_db = kcdbnew();
    if (!kcdbopen(master_db, "master.kch", KCOCREATE | KCOWRITER | KCOREADER)) {
        printf("master.kch open error: %s\n", kcecodename(kcdbecode(master_db)));
        return 1;
    }

    if (!kcdbiterate(master_pgn_db, visit_master_pgn, NULL, false)) {
        printf("master-pgn.kct iterate error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
    }

    if (!kcdbclose(master_pgn_db)) {
        printf("master-pgn.kct close error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
    }

    kcdbdel(master_pgn_db);
    return 0;
}
