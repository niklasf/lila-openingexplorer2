from __future__ import print_function

import chess

print("#ifndef BITBOARD_H_")
print("#define BITBOARD_H_")
print()
print("#include <stdint.h>")

print()

print("#define BB_VOID 0x%016xULL" % chess.BB_VOID)
print("#define BB_ALL  0x%016xULL" % chess.BB_ALL)

print()

for square, name in enumerate(chess.SQUARE_NAMES):
    bb = chess.BB_SQUARES[square]
    print("#define BB_%s 0x%016xULL" % (name.upper(), bb))

print()
print("#define BB_SQUARE(sq) (1ULL << sq)")
print()

for rank, bb in enumerate(chess.BB_RANKS):
    print("#define BB_RANK_%d 0x%016xULL" % (rank + 1, bb))

print()
print("#define BB_BACKRANKS (BB_RANK_1 | BB_RANK_8)")
print()

for name, bb in zip(chess.FILE_NAMES, chess.BB_FILES):
    print("#define BB_FILE_%s 0x%016xULL" % (name.upper(), bb))

print()

print("#endif  // ifndef BITBOARD_H_")
