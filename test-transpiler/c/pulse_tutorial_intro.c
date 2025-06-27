#include <stdint.h>
#include <stdlib.h>
#include "pulse_macros.h"

REQUIRES("x |-> 'i")
REQUIRES("pure FStar.Int32.(fits (v 'i + 1))")
ENSURES("x |-> ('i + 1)")
void incr (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(i)
REQUIRES(x |-> i)
REQUIRES(pure FStar.Int32.(fits (v i + 1)))
ENSURES(x |-> (i + 1))
void incr_explicit_i (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(i)
ERASED_ARG(j)
REQUIRES(x |-> i ** y |-> j)
REQUIRES(pure FStar.Int32.(fits (v i + 1)))
ENSURES(x |-> (i + 1) ** y |-> j)
void incr_frame(int *x, int *y)
{
    incr(x);
}