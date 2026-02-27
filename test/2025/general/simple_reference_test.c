#include "../include/PulseMacros.h"

// This works but returns a dead ref
RETURNS(s:ref Pulse.Lib.C.Int32.int32)
// ENSURES(pure Pulse.Lib.C.Int32.(as_int s == 0))
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}
