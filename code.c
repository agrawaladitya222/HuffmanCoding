#include "code.h"

#include "defines.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* typedef struct Code {
    uint32_t top;
    uint8_t bits[MAX_CODE_SIZE];
} Code; */

Code code_init(void) {
    Code c;
    c.top = 0;
    for (uint32_t i = 0; i < MAX_CODE_SIZE; i += 1) {
        c.bits[i] = 0;
    }
    return c;
}

uint32_t code_size(Code *c) {
    return c->top;
}

bool code_empty(Code *c) {
    return (c->top == 0);
}

bool code_full(Code *c) {
    return (c->top == ALPHABET);
}

bool code_push_bit(Code *c, uint8_t bit) {
    if (code_full(c)) {
        return false;
    }
    c->bits[c->top / 8] |= (bit << (c->top % 8));
    c->top += 1;
    return true;
}

bool code_pop_bit(Code *c, uint8_t *bit) {
    if (code_empty(c)) {
        return false;
    }
    c->top -= 1;
    /**bit = (1 << (c->top % 8));
    *bit &= c->bits[c->top / 8];
    *bit = (*bit >> (c->top % 8));
    */
    *bit = (c->bits[c->top/8] >> (c->top % 8)) & 1;
    return true;
}

void code_print(Code *c) {
    for (uint32_t i = 0; i < c->top; i += 1) {
        fprintf(stderr, "%" PRIu8 " ", ((c->bits[i / 8] >> (i % 8)) & 1));
    }
    fprintf(stderr, "\n");
    return;
}
