#include "pal.h"
#include <stdbool.h>

_let(bool spec_id(bool x), x)

void test_let(bool *p)
    _requires(*p == true)
    _ensures(*p == true)
{
    _assert(spec_id(*p) == true);
}
