#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(r |-> vr))
_requires((_slprop) _inline_pulse(pure Pulse.Lib.C.Int32.(fits (+) (as_int vr) 1)))
_ensures((_slprop) _inline_pulse(exists* w. (r |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == as_int vr + 1)))
int incr (int *r)
{
    *r = *r + 1;
    return *r;
}

_ensures((_slprop) _inline_pulse(pure Pulse.Lib.C.Int32.(as_int i == 1)))
int one()
{
    int i = 0;
    incr(&i);
    return i;
}
