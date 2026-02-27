#include <stdint.h>
#include "../include/PulseMacros.h"

typedef struct _s1 {
    ISARRAY(32) uint8_t *bytes;
} s1;

PRESERVES(exists* s1_s y. s1_pred x1 s1_s ** (s1_s.bytes |-> y))
uint8_t read_s10 (s1 *x1)
{
    LEMMA(s1_explode (!x1));
    LEMMA(pts_to_len (!(!(!x1)).bytes));
    uint8_t res = x1->bytes[0];
    LEMMA(admit());
    return 0;
}
