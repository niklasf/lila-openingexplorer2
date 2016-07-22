#include "move.h"

char *move_uci(move_t move, char *uci) {
    if (!move) {
        *uci++ = '0';
        *uci++ = '0';
        *uci++ = '0';
        *uci++ = '0';
    } else if (move_is_drop(move)) {
        *uci++ = move_piece_type(move) - 'a' + 'A';
        *uci++ = '@';
        uci = square_name(move_to(move), uci);
    } else {
        uci = square_name(move_from(move), uci);
        uci = square_name(move_to(move), uci);
        char piece_type = move_piece_type(move);
        if (piece_type) *uci++ = piece_type;
    }
    *uci = '\0';
    return uci;
}
