#include <string.h>
#include <stdlib.h>

#include "pgn.h"

struct pgn_game_info *pgn_game_info_read(char *pgn, char **saveptr_pgn) {
    struct pgn_game_info *game_info = calloc(1, sizeof(struct pgn_game_info));
    if (!game_info) abort();

    char *line = strtok_r(pgn, "\n", saveptr_pgn);

    for (; line && line[0] == '[' && line[strlen(line) - 1] == ']'; line = strtok_r(NULL, "\n", saveptr_pgn)) {
        if (0 == strcmp(line, "[Result \"1/2-1/2\"]")) game_info->result = 0;
        else if (0 == strcmp(line, "[Result \"1-0\"]")) game_info->result = 1;
        else if (0 == strcmp(line, "[Result \"0-1\"]")) game_info->result = -1;
        else if (1 == sscanf(line, "[WhiteElo \"%d\"]", &game_info->white_elo)) continue;
        else if (1 == sscanf(line, "[BlackElo \"%d\"]", &game_info->black_elo)) continue;
        else if (strncmp("[White \"", line, strlen("[White \"")) == 0 && line[strlen(line) - 2] == '"') {
            game_info->white = strndup(line + strlen("[White \""), strlen(line) - strlen("[White \"") - 2);
        }
        else if (strncmp("[Black \"", line, strlen("[Black \"")) == 0 && line[strlen(line) - 2] == '"') {
            game_info->black = strndup(line + strlen("[Black \""), strlen(line) - strlen("[Black \"") - 2);
        }
    }

    return game_info;
}

void pgn_game_info_free(struct pgn_game_info *game_info) {
    if (game_info->white) free(game_info->white);
    if (game_info->black) free(game_info->black);
    free(game_info);
}
