#include <stdio.h>
#include <string.h>

#include "attacks.h"
#include "board.h"
#include "bitboard.h"
#include "move.h"

unsigned long perft(const board_t *pos, unsigned depth) {
    if (depth < 1) return 1;


    move_t moves[255];
    move_t *end = board_legal_moves(pos, moves, BB_ALL, BB_ALL);

    unsigned long result = 0;

    for (move_t *current = moves; current < end; current++) {
        board_t pos_after = *pos;
        board_move(&pos_after, *current);
        result += perft(&pos_after, depth - 1);
    }

    return result;
}

unsigned long print_perfts(const board_t *pos, unsigned depth) {
    move_t moves[255];
    move_t *end = board_legal_moves(pos, moves, BB_ALL, BB_ALL);
    unsigned long total = 0;

    for (move_t *current = moves; current < end; current++) {
        board_t pos_after = *pos;
        board_move(&pos_after, *current);

        char uci[6];
        move_uci(*current, uci);

        unsigned long p = perft(&pos_after, depth - 1);
        printf("%s: %lu\n", uci, p);
        total += p;
    }

    printf("total: %lu\n", total);
    return total;
}

void test_position_4() {
    const char fen[] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    printf("test_position_4: %s\n", fen);

    // Position 4 from https://chessprogramming.wikispaces.com/Perft+Results.
    board_t pos;
    assert(board_set_fen(&pos, fen));
    print_perfts(&pos, 3);

    // The position after f1f2.
    board_move(&pos, move_make(SQ_F1, SQ_F2, 0));
    assert(perft(&pos, 2) == 1623);
}

void test_tricky() {
    puts("test_tricky");
    board_t pos;
    // Source: https://chessprogramming.wikispaces.com/Perft+Results

    // Position 2: Kiwipete by Peter McKenzie.
    puts("- kiwipete");
    board_set_fen(&pos, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    assert(perft(&pos, 1) == 48);
    assert(perft(&pos, 2) == 2039);
    assert(perft(&pos, 3) == 97862);
    assert(perft(&pos, 4) == 4085603);

    // Position 3.
    puts("- position 3");
    board_set_fen(&pos, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    assert(perft(&pos, 1) == 14);
    assert(perft(&pos, 2) == 191);
    assert(perft(&pos, 3) == 2812);
    assert(perft(&pos, 4) == 43238);
    assert(perft(&pos, 5) == 674624);

    // Position 4.
    puts("- position 4");
    board_set_fen(&pos, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    assert(perft(&pos, 1) == 6);
    assert(perft(&pos, 2) == 264);
    assert(perft(&pos, 3) == 9467);
    assert(perft(&pos, 4) == 422333);
    assert(perft(&pos, 5) == 15833292);

    // Position 5: http://www.talkchess.com/forum/viewtopic.php?t=42463
    puts("- position 5");
    board_set_fen(&pos, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    assert(perft(&pos, 1) == 44);
    assert(perft(&pos, 2) == 1486);
    assert(perft(&pos, 3) == 62379);
    assert(perft(&pos, 4) == 2103487);

    // Position 6 by Steven Edwards.
    puts("- position 6");
    board_set_fen(&pos, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    assert(perft(&pos, 1) == 46);
    assert(perft(&pos, 2) == 2079);
    assert(perft(&pos, 3) == 89890);
    assert(perft(&pos, 4) == 3894594);
}

void test_random_epd() {
    puts("test_random_epd");

    FILE *file = fopen("perft-random.epd", "r");
    assert(file >= 0);

    char line[256];
    board_t pos;
    while (fgets(line, 128, file)) {
        char c = line[0];
        if (!c || c == ' ' || c == '\n' || c == '#') continue;
        if (c == 'i') {  // id
            printf("- %s", line);
        } else if (c == 'e') {  // epd
            line[strlen(line) - 1] = 0;
            strcat(line, " 0 1");
            assert(board_set_fen(&pos, line + 4));
        } else if (c == 'p') {  // perft
            unsigned depth;
            unsigned long p;
            assert(sscanf(line, "perft %d %lu", &depth, &p) == 2);
            if (p < 100000) {
                unsigned long result = perft(&pos, depth);
                if (result != p) {
                    printf("expected %lu, got %lu\n", p, result);
                    abort();
                }
            }
        }
    }

    fclose(file);
}

int main() {
    attacks_init();

    test_random_epd();
    test_position_4();
    test_tricky();
    return 0;
}
