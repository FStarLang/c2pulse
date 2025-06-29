#include <stdint.h>
#include <stdlib.h>
#include "../pulse_macros.h"

REQUIRES(exists* s. r |-> s)
ENSURES(r |-> v)
void assign (int *r, int v)
{
    *r = v;
}

REQUIRES(exists* s. (r |-> s) ** pure FStar.Int32.(fits (2 * v s)))
ENSURES(exists* s. (r |-> s) ** pure FStar.Int32.(v s % 2 == 0))
void make_even(int *x)
{
    x := *x + *x;
}


REQUIRES(exists* w0. (x |-> w0) ** pure FStar.Int32.(fits (2 * v s)))
ENSURES(exists* w1. (x |-> w1) ** pure (w1 % 2 == 0))
void make_even_explicit (int *x)
{
  LEMMA(with w0. assert (pts_to x w0));
  *x = *x + *x;
  LEMMA(
    introduce
    exists* w1. pts_to x w1 ** pure (w1 % 2 == 0)
    with (w0 + w0)
  );
}


REQUIRES(
  exists* wx wy.
    (x |-> wx) **
    (y |-> wy) ** 
    pure FStar.Int32.(v wx % 2 == v wy % 2 /\ fits (v wx + v wy))
)
ENSURES (
  exists* wx wy. (x |-> wx) ** (y |-> wy) ** pure FStar.Int32.(v wx % 2 == 0)  
)
void make_even_explicit_alt (int *x, int *y)
{
  LEMMA(with wx wy. _);
  *x = *x + *y;
  LEMMA(introduce exists* nx ny. 
          (x |-> nx) ** (y |-> ny) ** pure FStar.Int32.(nx % 2 == 0)
        with (wx + wy) wy);
}

ERASED_ARG(v)
REQUIRES(x |-> v)
ENSURES(exists* w. (x |-> v) ** pure FStar.Int32.(v w % 2 == 0))
void call_make_even (int *x)
{
  make_even(x);
}