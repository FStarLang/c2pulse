#include <stdint.h>
#include "../include/PulseMacros.h"


REQUIRES(exists* v. x |-> v)
RETURNS(b:bool)
ENSURES(exists* v. x |-> v)
uint32_t count_down (uint32_t *x)
{
  return (*x == 0);
}


REQUIRES(exists* v. x |-> v)
RETURNS(b:FStar.UInt32.t)
ENSURES(exists* v. x |-> v)
uint32_t decr (uint32_t *x)
{
  return (*x - 0);
}