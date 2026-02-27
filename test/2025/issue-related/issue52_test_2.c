#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(exists* v. (x |-> v)))
_ensures((_slprop) _inline_pulse(exists* v. (x |-> v)))
int fake_arr(int* x) {
        int idx = 0;
        _assert((_slprop) _inline_pulse(pts_to_len !x));
        return x[idx];
}
