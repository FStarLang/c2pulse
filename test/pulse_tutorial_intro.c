#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

REQUIRES("x |-> 'i")
REQUIRES("pure FStar.Int32.(fits (v 'i + 1))")
ENSURES("exists* j. (x |-> j) ** pure FStar.Int32.(v 'i + 1 == v j)")
void incr (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(#i:_)
REQUIRES(x |-> i)
REQUIRES(pure FStar.Int32.(fits (v i + 1)))
ENSURES("exists* j. (x |-> j) ** pure FStar.Int32.(v i + 1 == v j)")
void incr_explicit_i (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(#i:_)
ERASED_ARG(#j:_)
REQUIRES(x |-> i)
REQUIRES(y |-> j)
REQUIRES(pure FStar.Int32.(fits (v i + 1)))
ENSURES("exists* j. (x |-> j) ** pure FStar.Int32.(v i + 1 == v j)")
ENSURES(y |-> j)
void incr_frame(int *x, int *y)
{
    incr(x);
}