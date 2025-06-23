// RUN: %c2pulse %s 2>&1 | %{FILECHECK} %s
// RUN: %run_fstar.sh %p/Swap_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include "../../test-transpiler/c/pulse_macros.h"

REQUIRES((r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2))
ENSURES((r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1))
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}

// CHECK: Print the Pulse function Definition:

// CHECK: The pulse function Name is: ref_swap

// CHECK: Print function arguments: (r1,ref Int32.t),(r2,ref Int32.t),

// CHECK: Print annotations: 

// CHECK: Requires (r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)
// CHECK: Ensures (r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)


// CHECK: Print the function body: 

// CHECK: let tmp = (! r1)
// CHECK: r1 := (! r2)
// CHECK: r2 := tmp

// CHECK: End printing the function Definition

// PULSE: Verified module: Swap
// PULSE-NEXT: All verification conditions discharged successfully