#include <stdbool.h>

#include <kclangc.h>

const char *visit_master_pgn(const char *game_id, size_t game_id_size,
                             const char *pgn, size_t pgn_size,
                             size_t *sp, void *opq) {
    assert(game_id_size == 8);

    char c_game_id[9];
    strncpy(c_game_id, game_id, 8);

    printf("%s\n", c_game_id);
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
