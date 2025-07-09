#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

REQUIRES(exists* v. (x |-> v) ** pure (as_int v >= 0))
ENSURES(x |-> 0l)
void count_down (int *x)
{
  int keep_going = 1;
  while (keep_going == 1)
  INVARIANTS(invariant b.
    exists* k v.
      (keep_going |-> k) **
      (x |-> v) **
      pure (as_int v >= 0) **
      pure (b==(k=1l)) **
      pure (k<>1l ==> v==0l)
  )
  {
    if (*x == 0)
    {
      keep_going = 0;
    }
    else
    {
      *x = *x - 1;
    }
  }
}

REQUIRES(pure (fits ( * ) (as_int x) (as_int y)))
REQUIRES(pure (as_int x >= 0))
RETURNS(i:int32)
ENSURES(pure (as_int i == as_int x * as_int y))
int multiply_by_repeated_addition (int x, int y)
{
  int ctr = 0;
  int acc = 0;
  while (ctr < x)
  INVARIANTS(invariant b. 
    exists* c a.
      (ctr |-> c) **
      (acc |-> a) **
      pure (as_int c <= as_int x) **
      pure (as_int a == (as_int c * as_int y)) **
      pure (b == (as_int c < as_int x))
  )
  {
    ctr = ctr + 1;
    acc = acc + y;
  }
  return acc;
}