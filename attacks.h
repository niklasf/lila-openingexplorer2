#ifndef ATTACKS_H_
#define ATTACKS_H_

#include <stdint.h>

void attacks_init();

uint64_t attacks_rook(uint8_t square, uint64_t occupied);
uint64_t attacks_bishop(uint8_t square, uint64_t occupied);

#endif  // #ifndef ATTACKS_H_
