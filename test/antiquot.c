#include "c2pulse.h"

void test_rvalue_antiquot(int *x)
    _ensures((_slprop) _inline_pulse(pure (Int32.v $(*x) > 0)))
{
    _assert((_slprop) _inline_pulse($&(x) |-> $(x) ** $&(*x) |-> $(*x)));
    *x = 6 + 7;
}

void test_ghost_stmt(int *x)
    _requires(*x == (int) 0)
    _ensures(*x == (int) 0)
{
    _ghost_stmt(rewrite (pure ($(*x) = $(*x))) as (pure ($(*x) = $(*x))));
    return;
}
