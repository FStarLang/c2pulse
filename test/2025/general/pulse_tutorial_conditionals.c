#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_include_pulse(
  let max_spec x y = if x < y then y else x
)

_requires((_slprop) _inline_pulse(x |-> Frac px vx))
_requires((_slprop) _inline_pulse(y |-> Frac py vy))
_ensures((_slprop) _inline_pulse(x |-> Frac px vx))
_ensures((_slprop) _inline_pulse(y |-> Frac py vy))
_ensures((_slprop) _inline_pulse(pure (as_int n == max_spec (as_int vx) (as_int vy))))
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

_requires((_slprop) _inline_pulse(x |-> Frac px vx))
_requires((_slprop) _inline_pulse(y |-> Frac py vy))
_ensures((_slprop) _inline_pulse(x |-> Frac px vx))
_ensures((_slprop) _inline_pulse(y |-> Frac py vy))
_ensures((_slprop) _inline_pulse(pure (as_int n == max_spec (as_int vx) (as_int vy))))
int max_alt(int *x, int *y)
{
  int result = 0; 
  int vx = *x;
  int vy = *y;
  if (vx > vy)
  _ensures((_slprop) _inline_pulse(exists* r.
       (x |-> Frac px vx) **
       (y |-> Frac py vy) **
       (result |-> r) **
       pure (as_int r == max_spec (as_int vx) (as_int vy))))
  {
    result = vx;
  }
  else
  {
    result = vy;
  };
  return result;
}

_requires((_slprop) _inline_pulse(r |->? Frac p w))
_ensures((_slprop) _inline_pulse(r |->? Frac p w))
_ensures((_slprop) _inline_pulse(pure (Some? w ==> Some?.v w == i)))
int read_nullable(int *r)
{
  if (r == NULL)
  {
    _assert((_slprop) _inline_pulse(elim_intro_null !r));
    return 0;
  }
  else
  {
    _assert((_slprop) _inline_pulse(elim_non_null !r));
    int v = *r;
    _assert((_slprop) _inline_pulse(intro_non_null !r));
    return v;
  }
}

_requires((_slprop) _inline_pulse(r |->? w))
_ensures((_slprop) _inline_pulse(exists* x. (r |->? x) ** pure (if Some? w then x == Some v else x == w)))
void write_nullable(int *r, int v)
{
  if (r == NULL)
  {
    _assert((_slprop) _inline_pulse(elim_intro_null !r));
  }
  else
  {
    _assert((_slprop) _inline_pulse(elim_non_null !r));
    *r = v;
    _assert((_slprop) _inline_pulse(intro_non_null !r));
  }
}