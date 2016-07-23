#include <stdbool.h>

#include <kclangc.h>

const char *visit_master_pgn(const char *game_id, size_t game_id_size,
                             const char *buf, size_t buf_size,
                             size_t *sp, void *opq) {
    char *pgn = strndup(buf, buf_size);
    char *saveptr_pgn, *saveptr_line;

    char *line = strtok_r(pgn, "\n", &saveptr_pgn);

    // Parse headers.
    while (line && line[0] == '[') {
        printf("%s\n", line);
        line = strtok_r(NULL, "\n", &saveptr_pgn);
    }

    // Parse movetext.
    while (line) {
        char *token = strtok_r(line, " ", &saveptr_line);
        while (token) {
            printf("  -> %s <-\n", token);
            token = strtok_r(NULL, " ", &saveptr_line);
        }

        line = strtok_r(NULL, "\n", &saveptr_pgn);
    }

    free(pgn);
    abort();
    return KCVISNOP;
}

int main() {
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
