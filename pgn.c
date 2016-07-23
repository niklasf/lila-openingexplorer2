#include <string.h>

#include "pgn.h"

void pgn_game_info_reset(struct pgn_game_info *game_info) {
    strcpy(game_info->Event, "?");
    strcpy(game_info->Site, "?");
    strcpy(game_info->Date, "????.??.??");
    strcpy(game_info->Round, "?");
    strcpy(game_info->White, "?");
    strcpy(game_info->Black, "?");

    game_info->WhiteElo = 0;
    game_info->BlackElo = 0;

    game_info->year = 0;
    game_info->result = 0;
}

bool pgn_read_game(FILE *file, struct pgn_game_info *game_info) {
    pgn_game_info_reset(game_info);
    return false;
}
