#include "c2pulse.h"

void test_rvalue_antiquot(int *x)
    _ensures((_slprop) _inline_pulse(pure ($(*x) = $(*x))))
{
    return;
}
