#include "c2pulse.h"
#include <stdbool.h>

int test_implies(bool a, bool b)
    _requires(a ==> b)
    _ensures(a ==> b)
{
    return 0;
}
