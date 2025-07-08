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

ERASED_ARG(#w:option int32)
ERASED_ARG(#p:_)
REQUIRES(r |->? Frac p w)
RETURNS(i:int32)
ENSURES(r |->? Frac p w)
ENSURES(pure (Some? w ==> Some?.v w == i))
int read_nullable(int *r)
{
  if (r == NULL)
  {
    LEMMA(elim_intro_null r);
    return 0;
  }
  else
  {
    LEMMA(elim_non_null r);
    int v = *r;
    LEMMA(intro_non_null r);
    return v;
  }
}

ERASED_ARG(#w:option int32)
REQUIRES(r |->? w)
ENSURES(exists* x. (r |->? x) ** pure (if Some? w then x == Some v else x == w))
void write_nullable(int *r, int v)
{
  if (r == NULL)
  {
    LEMMA(elim_intro_null r);
  }
  else
  {
    LEMMA(elim_non_null r);
    *r = v;
    LEMMA(intro_non_null r);
  }
}