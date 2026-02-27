#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(pure FStar.Int32.(fits (v (!x) + 1))))
_ensures((_slprop) _inline_pulse(pure FStar.Int32.(eq2 #int (v (!x)) (v (old (!x)) + 1))))
void incr (int *x)
{
    *x = *x + 1;
}

_requires((_slprop) _inline_pulse(x |-> i))
_requires((_slprop) _inline_pulse(y |-> j))
_requires((_slprop) _inline_pulse(pure FStar.Int32.(fits (v i + 1))))
_ensures((_slprop) _inline_pulse(exists* k. (x |-> k) ** pure FStar.Int32.(v i + 1 == v k)))
_ensures((_slprop) _inline_pulse(y |-> j))
void incr_frame(int *x, int *y)
{
    incr(x);
}
