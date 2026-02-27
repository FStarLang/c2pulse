#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

PRESERVES(live x)
REQUIRES(pure FStar.Int32.(fits (v (!x) + 1)))
ENSURES(pure FStar.Int32.(eq2 #int (v (!x)) (v (old (!x)) + 1)))
void incr (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(#i:_)
ERASED_ARG(#j:_)
REQUIRES(x |-> i)
REQUIRES(y |-> j)
REQUIRES(pure FStar.Int32.(fits (v i + 1)))
ENSURES(exists* k. (x |-> k) ** pure FStar.Int32.(v i + 1 == v k))
ENSURES(y |-> j)
void incr_frame(int *x, int *y)
{
    incr(x);
}
