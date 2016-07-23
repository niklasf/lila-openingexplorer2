#include <stdbool.h>

#include <kclangc.h>

#include "attacks.h"
#include "board.h"

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
            board_print(&pos);
            puts("--");

            move_t move;
            if (board_parse_san(&pos, token, &move)) {
                board_move(&pos, move);
            }
        }

        line = strtok_r(NULL, "\n", &saveptr_pgn);
    }

    free(pgn);
    abort();
    return KCVISNOP;
}

int main() {
    attacks_init();

    KCDB *master_pgn_db = kcdbnew();
    if (!kcdbopen(master_pgn_db, "master-pgn.kct", KCOREADER)) {
        printf("master-pgn.kct open error: %s\n", kcecodename(kcdbecode(master_pgn_db)));
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
