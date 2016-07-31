#ifndef PGN_H_
#define PGN_H_

#include <stdio.h>
#include <stdbool.h>

struct pgn_game_info {
    char *white;
    char *black;

    int white_elo;
    int black_elo;

    unsigned year;

    int result;
};

struct pgn_game_info *pgn_game_info_read(char *pgn, char **saveptr_pgn);

void pgn_game_info_free(struct pgn_game_info *game_info);

#endif  // #ifndef PGN_H_
