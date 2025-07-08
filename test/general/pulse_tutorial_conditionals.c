#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

INCLUDE(
  let max_spec x y = if x < y then y else x
)

ERASED_ARG(#vx #vy:_)
ERASED_ARG(#px #py:_)
REQUIRES(x |-> Frac px vx)
REQUIRES(y |-> Frac py vy)
RETURNS(n:int32)
ENSURES(x |-> Frac px vx)
ENSURES(y |-> Frac py vy)
ENSURES(pure (as_int n == max_spec (as_int vx) (as_int vy)))
int max (int *x, int *y)
{
    if (*x > *y)
    {
        return *x;
    }
    else 
    {
        return *y;
    }
}


ERASED_ARG(#vx #vy:_)
ERASED_ARG(#px #py:_)
REQUIRES(x |-> Frac px vx)
REQUIRES(y |-> Frac py vy)
RETURNS(n:int32)
ENSURES(x |-> Frac px vx)
ENSURES(y |-> Frac py vy)
ENSURES(pure (as_int n == max_spec (as_int vx) (as_int vy)))
int max_alt(int *x, int *y)
{
  int result = 0; 
  int vx = *x;
  int vy = *y;
  if (vx > vy)
  ENSURES(
    exists* r.
       (x |-> Frac px vx) **
       (y |-> Frac py vy) **
       (result |-> r) **
       pure (as_int r == max_spec (as_int vx) (as_int vy))
  )
  {
    result = vx;
  }
  else
  {
    result = vy;
  };
  return result;
}
