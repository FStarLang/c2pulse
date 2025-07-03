// RUN: %c2pulse %s
// RUN: cat %p/Pulse_tutorial_existentials.fst
// RUN: diff %p/Pulse_tutorial_existentials.fst %p/../snapshots/Pulse_tutorial_existentials.fst
// RUN: %run_fstar.sh %p/Pulse_tutorial_existentials.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE
#include <stdint.h>
#include <stdlib.h>
#include "../../include/PulseMacros.h"

REQUIRES(exists* s. r |-> s)
ENSURES(r |-> v)
void assign (int *r, int v)
{
    *r = v;
}

REQUIRES(exists* s. (x |-> s) ** pure (fits ( * ) 2 (as_int s)))
ENSURES(exists* s. (x |-> s) ** pure (as_int s % 2 == 0))
void make_even(int *x)
{
    *x = *x + *x;
}


REQUIRES(exists* w0. (x |-> w0) ** pure (fits ( * ) 2 (as_int w0)))
ENSURES(exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0))
void make_even_explicit (int *x)
{
  LEMMA(with w0. assert (x |-> w0));
  *x = *x + *x;
  LEMMA(
    introduce
    exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0)
    with (w0 +^ w0)
  );
}


REQUIRES(exists* wx wy.  (x |-> wx) ** (y |-> wy) ** pure (as_int wx % 2 == as_int wy % 2) ** pure (fits (+) (as_int wx) (as_int wy)))
ENSURES(exists* wx wy. (x |-> wx) ** (y |-> wy))
void make_even_explicit_alt (int *x, int *y)
{
  LEMMA(with wx wy. assert ((x |-> wx) ** (y |-> wy)));
  *x = *x + *y;
  LEMMA(introduce exists* nx ny. 
          (x |-> nx) ** (y |-> ny) ** pure (as_int nx % 2 == 0)
        with (wx +^ wy) wy);
}

ERASED_ARG(#v:erased _)
REQUIRES(x |-> v)
REQUIRES(pure (fits (+) (as_int v) (as_int v)))
ENSURES(exists* w. (x |-> w) ** pure (as_int w % 2 == 0))
void call_make_even (int *x)
{
  make_even(x);
}


// PULSE: All verification conditions discharged successfully
