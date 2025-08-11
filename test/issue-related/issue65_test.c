#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

typedef struct ab_struct {
    ISARRAY(24) uint8_t *a;
    ISARRAY(48) uint8_t *b;
} ab_struct;

typedef union ab_union {
    ISARRAY(24) uint8_t *a;
    ISARRAY(48) uint8_t *b;
} ab_union;
