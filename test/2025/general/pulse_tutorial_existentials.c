#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(exists* s. r |-> s))
_ensures((_slprop) _inline_pulse(r |-> v))
void assign (int *r, int v)
{
    *r = v;
}

_requires((_slprop) _inline_pulse(exists* s. (x |-> s) ** pure (fits ( * ) 2 (as_int s))))
_ensures((_slprop) _inline_pulse(exists* s. (x |-> s) ** pure (as_int s % 2 == 0)))
void make_even(int *x)
{
    *x = *x + *x;
}

_requires((_slprop) _inline_pulse(exists* w0. (x |-> w0) ** pure (fits ( * ) 2 (as_int w0))))
_ensures((_slprop) _inline_pulse(exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0)))
void make_even_explicit(int *x)
{
  _assert((_slprop) _inline_pulse(with w0. assert (x |-> w0)));
  *x = *x + *x;
  _assert((_slprop) _inline_pulse(introduce
    exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0)
    with (w0 +^ w0)));
}

_requires((_slprop) _inline_pulse(exists* wx wy.  (x |-> wx) ** (y |-> wy) ** pure (as_int wx % 2 == as_int wy % 2) ** pure (fits (+) (as_int wx) (as_int wy))))
_ensures((_slprop) _inline_pulse(exists* wx wy. (x |-> wx) ** (y |-> wy)))
void make_even_explicit_alt (int *x, int *y)
{
  _assert((_slprop) _inline_pulse(with wx wy. assert ((x |-> wx) ** (y |-> wy))));
  *x = *x + *y;
  _assert((_slprop) _inline_pulse(introduce exists* nx ny. 
          (x |-> nx) ** (y |-> ny) ** pure (as_int nx % 2 == 0)
        with (wx +^ wy) wy));
}

_requires((_slprop) _inline_pulse(x |-> v))
_requires((_slprop) _inline_pulse(pure (fits (+) (as_int v) (as_int v))))
_ensures((_slprop) _inline_pulse(exists* w. (x |-> w) ** pure (as_int w % 2 == 0)))
void call_make_even (int *x)
{
  make_even(x);
}