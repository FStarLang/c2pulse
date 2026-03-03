#include "c2pulse.h"

void test_rvalue_antiquot(int *x)
    _ensures((_slprop) _inline_pulse(pure (Int32.v $(*x) > 0)))
{
    _assert((_slprop) _inline_pulse($&(x) |-> $(x)));
    *x = 6 + 7;
}
