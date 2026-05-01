#include "pal.h"
#include <stdbool.h>

_let(bool spec_id(bool x), x)

_let(_specint double_spec(_specint x), x + x)

void test_let(bool *p)
    _requires(*p == true)
    _ensures(*p == true)
{
    _assert(spec_id(*p) == true);
    _assert(double_spec(3) == 6);
}
