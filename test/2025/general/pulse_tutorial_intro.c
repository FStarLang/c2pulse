#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(x |-> i))
_requires((_slprop) _inline_pulse(pure (fits (+) (as_int i) 1)))
_ensures((_slprop) _inline_pulse(exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)))
void incr (int *x)
{
    *x = *x + 1;
}

_requires((_slprop) _inline_pulse(x |-> i))
_requires((_slprop) _inline_pulse(y |-> j))
_requires((_slprop) _inline_pulse(pure (fits (+) (as_int i) 1)))
_ensures((_slprop) _inline_pulse(exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)))
_ensures((_slprop) _inline_pulse(y |-> j))
void incr_frame(int *x, int *y)
{
    incr(x);
}