#include "../include/PulseMacros.h"
#include <stdint.h>
#include <stdlib.h>

INCLUDE(let max_spec x y = if x < y then y else x)

ERASED_ARG(#vx #vy : erased _)
ERASED_ARG(#px #py : _)
PRESERVES(x |->Frac px vx)
PRESERVES(y |->Frac py vy)
RETURNS(n : int32)
ENSURES(pure(as_int n == max_spec(as_int vx)(as_int vy)))
int max(int *x, int *y) {
  if (*x > *y) {
    return *x;
  } else {
    return *y;
  }
}