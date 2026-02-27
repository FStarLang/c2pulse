#include "../c2pulse.h"

// This works but returns a dead ref
// _ensures((_slprop) _inline_pulse(pure Pulse.Lib.C.Int32.(as_int s == 0)))
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}
