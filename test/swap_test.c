// RUN: %c2pulse %s 2>&1 | %{FILECHECK} %s

#include "../../test-transpiler/c/pulse_macros.h"

REQUIRES((r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2))
ENSURES((r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1))
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}

// CHECK: The pulse function Name is: ref_swap

// CHECK: Print Program:

// CHECK: fn ref_swap
// CHECK-NEXT: (r1 : ref Int32.t)
// CHECK-NEXT: (r2 : ref Int32.t)
// CHECK: {
// CHECK-NEXT: let tmp = (! r1);
// CHECK-NEXT: r1 := (! r2);
// CHECK-NEXT: r2 := tmp;
// CHECK: }

// CHECK: Success: Code transformed and syntax validated.