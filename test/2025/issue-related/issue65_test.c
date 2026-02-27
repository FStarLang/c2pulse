#include <stdint.h>
#include <stdbool.h>
#include "../c2pulse.h"

typedef struct ab_struct {
    uint8_t *a;
    uint8_t *b;
} ab_struct;

typedef union ab_union {
    uint8_t *a;
    uint8_t *b;
} ab_union;
