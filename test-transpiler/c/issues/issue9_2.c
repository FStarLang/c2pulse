#include "../../../include/PulseMacros.h"
ERASED_ARG(#vr:erased _)
REQUIRES(r |-> vr)
REQUIRES(pure Pulse.Lib.C.Int32.(fits (+) (as_int vr) 1))
ENSURES(exists* w. (r |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == as_int vr + 1))
int incr (int *r)
{
    *r = *r + 1;
}

REQUIRES(emp)
RETURNS(i:Pulse.Lib.C.Int32.int32)
ENSURES(pure Pulse.Lib.C.Int32.(as_int i == 1))
int one()
{
    int i = 0;
    incr(&i);
    return i;
}
