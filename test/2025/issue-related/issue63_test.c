#include <stdint.h>
#include "../c2pulse.h"

typedef struct _s1 {
    uint8_t *bytes;
} s1;

_preserves((_slprop) _inline_pulse(exists* s1_s y. s1_pred x1 s1_s ** (s1_s.bytes |-> y)))
uint8_t read_s10 (s1 *x1)
{
    _assert((_slprop) _inline_pulse(s1_explode (!x1)));
    _assert((_slprop) _inline_pulse(pts_to_len (!(!(!x1)).bytes)));
    uint8_t res = x1->bytes[0];
    _assert((_slprop) _inline_pulse(admit()));
    return 0;
}
