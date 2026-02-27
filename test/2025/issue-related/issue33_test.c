#include <stdint.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(exists* v. x |-> v))
_ensures((_slprop) _inline_pulse(exists* v. x |-> v))
uint32_t count_down (uint32_t *x)
{
  return (*x == 0);
}

_requires((_slprop) _inline_pulse(exists* v. x |-> v))
_ensures((_slprop) _inline_pulse(exists* v. x |-> v))
uint32_t decr (uint32_t *x)
{
  return (*x - 0);
}