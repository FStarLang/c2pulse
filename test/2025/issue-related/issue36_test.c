#include "../include/PulseMacros.h"

REQUIRES(pure (as_int x > 0))
void test(int x)
{
    x = x - 1;
}
