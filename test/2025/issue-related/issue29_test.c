#include "../c2pulse.h"
#include <stdint.h>
#include <stdlib.h>

_include_pulse(let max_spec x y = if x < y then y else x)

_preserves((_slprop) _inline_pulse(x |->Frac px vx))
_preserves((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(pure(as_int n == max_spec(as_int vx)(as_int vy))))
int max(int *x, int *y) {
  if (*x > *y) {
    return *x;
  } else {
    return *y;
  }
}