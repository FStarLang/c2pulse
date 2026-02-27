#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(pure (as_int x > 0)))
void test(int x)
{
    x = x - 1;
}
