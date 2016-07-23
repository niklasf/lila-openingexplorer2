#include <stdio.h>
#include <assert.h>

#include "attacks.h"
#include "pgn.h"

void test_pgn_read_game() {
    puts("test_pgn_read_game");

    FILE *file = fopen("molinari-bordais-1979.pgn", "r");
    struct pgn_game_info game_info;
    assert(pgn_read_game(file, &game_info));
    // printf("w: %s, b: %s", game_info.white, game_info.black);

    fclose(file);
}

int main() {
    attacks_init();

    test_pgn_read_game();
    return 0;
}
