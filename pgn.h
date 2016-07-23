#ifndef PGN_H_
#define PGN_H_

#include <stdio.h>
#include <stdbool.h>

struct pgn_game_info {
    char Event[255];
    char Site[255];
    char Date[255];
    char Round[255];
    char White[255];
    char Black[255];

    unsigned WhiteElo;
    unsigned BlackElo;

    unsigned year;
    char result;
};

void pgn_game_info_reset(struct pgn_game_info *game_info);

bool pgn_read_game(FILE *file, struct pgn_game_info *game_info);

#endif  // #ifndef PGN_H_
