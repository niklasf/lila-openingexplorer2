#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "attacks.h"
#include "pgn.h"

void test_pgn_read_game() {
    puts("test_pgn_read_game");
    char line[] = "[White \"A person\"]\n";

    char *save_ptr;
    struct pgn_game_info *game_info = pgn_game_info_read(line, &save_ptr);

    if (game_info->white) puts(game_info->white);
    else puts("NULL");

    assert(strcmp(game_info->white, "A person") == 0);

    pgn_game_info_free(game_info);
}

int main() {
    attacks_init();

    test_pgn_read_game();
    return 0;
}
