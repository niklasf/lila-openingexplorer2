#include <assert.h>

#include "board.h"

void test_set_fen() {
    struct board pos;
    assert(board_set_fen(&pos, "8/8/3k4/8/1q4N1/6K1/1p6/4R3 w - - 0 1"));
    assert((pos.white & pos.queens) == 0);
}

int main() {
    test_set_fen();
}
