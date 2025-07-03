// RUN: %c2pulse %s
// RUN: cat %p/Issue1_test.fst
// RUN: diff %p/Issue1_test.fst %p/../snapshots/Issue1_test.fst
// RUN: %run_fstar.sh %p/Issue1_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE
#include <stdint.h>
#include <stdlib.h>
#include "../../include/PulseMacros.h"

REQUIRES("x |-> 'i")
REQUIRES("pure FStar.Int32.(fits (v 'i + 1))")
ENSURES("exists* j. (x |-> j) ** pure FStar.Int32.((v j <: int) == v 'i + 1)")
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



// PULSE: All verification conditions discharged successfully
