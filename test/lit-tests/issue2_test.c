// RUN: %c2pulse %s
// RUN: cat %p/Issue2_test.fst
// RUN: diff %p/Issue2_test.fst %p/../snapshots/Issue2_test.fst
// RUN: %run_fstar.sh %p/Issue2_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE
#include "../../include/PulseMacros.h"
	
ERASED_ARG(#w:_)
REQUIRES(r |-> w)
RETURNS(v:Int32.t)
ENSURES(r |-> w)
ENSURES(pure (v == w))
int value_of_explicit(int *r)
{
    return *r;
}



// PULSE: All verification conditions discharged successfully
