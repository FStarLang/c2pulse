#include "../c2pulse.h"

_ensures((_slprop) _inline_pulse(s |-> 0l))
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}