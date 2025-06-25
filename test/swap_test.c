// RUN: %c2pulse %s 
// RUN: cat %p/Swap_test/Module_0.fst | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Swap_test/Module_0.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include "../../test-transpiler/c/pulse_macros.h"

REQUIRES((r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2))
ENSURES((r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1))
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}

// C2PULSE: module Module_0
// C2PULSE: #lang-pulse
// C2PULSE: open Pulse

// C2PULSE: fn ref_swap
// C2PULSE: (r1 : ref Int32.t)
// C2PULSE: (r2 : ref Int32.t)
// C2PULSE: requires (r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)
// C2PULSE: ensures (r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)
// C2PULSE: {
// C2PULSE: let tmp = (! r1);
// C2PULSE: r1 := (! r2);
// C2PULSE: r2 := tmp;
// C2PULSE: }

// PULSE: All verification conditions discharged successfully