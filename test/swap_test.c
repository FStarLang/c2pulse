// RUN: %c2pulse %s 
// RUN: cat %p/Swap_test.fst 
// RUN: diff %p/Swap_test.fst %p/snapshots/Swap_test.fst
// RUN: %run_fstar.sh %p/Swap_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include "../include/PulseMacros.h"

REQUIRES((r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2))
ENSURES((r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1))
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}

// PULSE: All verification conditions discharged successfully
