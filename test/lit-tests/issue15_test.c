// RUN: %c2pulse %s
// RUN: cat %p/Issue15_test.fst
// RUN: diff %p/Issue15_test.fst %p/../snapshots/Issue15_test.fst
// RUN: %run_fstar.sh %p/Issue15_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE
#include "../../include/PulseMacros.h"

REQUIRES(emp)
ENSURES(emp)
void test_empty(void)
{ return;  }



// PULSE: All verification conditions discharged successfully
