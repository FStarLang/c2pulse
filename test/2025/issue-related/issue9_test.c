// EXPECT_FAILURE(19)
#include "../include/PulseMacros.h"

RETURNS(s:ref Pulse.Lib.C.Int32.int32)
// ENSURES(pure Pulse.Lib.C.Int32.(as_int s == 0))
// Now works but returns a dead ref
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}
