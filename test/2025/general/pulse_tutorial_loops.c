#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(exists* v. (x |-> v) ** pure (as_int v >= 0)))
_ensures((_slprop) _inline_pulse(x |-> 0l))
void count_down (int *x)
{
  int keep_going = 1;
  _assert((_slprop) _inline_pulse(with vx. assert (x |-> vx)));
  while (keep_going == 1)
      _invariant((_slprop) _inline_pulse(exists* k v.
      (keep_going |-> k) **
      (x |-> vx) ** (* tedious *)
      (vx |-> v) **
      pure (as_int v >= 0) **
      pure (b==(k=1l)) **
      pure (k<>1l ==> v==0l)))
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
_include_pulse( 
  module U32 = Pulse.Lib.C.UInt32
)

_requires((_slprop) _inline_pulse(pure (U32.fits ( * ) (U32.as_int x) (U32.as_int y))))
_ensures((_slprop) _inline_pulse(pure (U32.as_int i == U32.as_int x * U32.as_int y)))
uint32_t multiply_by_repeated_addition (uint32_t x, uint32_t y)
{
  uint32_t ctr = 0;
  uint32_t acc = 0;
  _assert((_slprop) _inline_pulse(with vx vy. assert (x |-> vx) ** (y |-> vy)));
  while (ctr < x)
      _invariant((_slprop) _inline_pulse(exists* c a.
      (ctr |-> c) **
      (acc |-> a) **
    pure (U32.as_int c <= U32.as_int vx) **
    pure (U32.as_int a == U32.as_int c * U32.as_int vy)))
  {
    ctr = ctr + 1;
    acc = acc + y;
  }
  return acc;
}

_requires((_slprop) _inline_pulse(pure (U32.fits ( * ) (U32.as_int x) (U32.as_int y))))
_ensures((_slprop) _inline_pulse(pure (U32.as_int i == U32.as_int x * U32.as_int y)))
uint32_t multiply_by_repeated_addition2 (uint32_t x, uint32_t y)
{
  uint32_t ctr = 0;
  uint32_t acc = 0;
  while (ctr < x)
      _invariant(_live(ctr))
    _invariant(_live(acc))
    _invariant((_slprop) _inline_pulse(pure U32.(as_int !ctr <= as_int !x)))
    _invariant((_slprop) _inline_pulse(pure U32.(as_int !acc == U32.as_int !ctr * U32.as_int !y)))
  {
    ctr = ctr + 1;
    acc = acc + y;
  }
  return acc;
}

_include_pulse(
let rec sum (n:nat)
: nat
= if n = 0 then 0 else n + sum (n - 1)

let rec sum_lemma (n:nat)
: Lemma (sum n == n * (n + 1) / 2)
= if n = 0 then ()
  else sum_lemma (n - 1)

let sum_mono (c n:nat)
: Lemma
  (requires c <= n)
  (ensures sum c <= sum n)
  [SMTPat (sum c); SMTPat (sum n)]
= sum_lemma c; sum_lemma n
)

_requires((_slprop) _inline_pulse(pure (as_int n >= 0)))
_requires((_slprop) _inline_pulse(pure (fits (+) (as_int n) 1)))
_requires((_slprop) _inline_pulse(pure (fits ( * ) (as_int n) (as_int n + 1))))
_ensures((_slprop) _inline_pulse(pure (as_int i == (as_int n * (as_int n + 1)) / 2)))
int isum (int n)
{
  int acc = 0;
  int ctr = 0;
  _assert((_slprop) _inline_pulse(sum_lemma(as_int !n)));
  _assert((_slprop) _inline_pulse(with vn. assert (n |-> vn)));
  while (ctr < n)
      _invariant((_slprop) _inline_pulse(exists* c a.
      (n |-> vn) ** (* tedious *)
      (ctr |-> c) **
      (acc |-> a) **
      pure (as_int c <= as_int vn) **
      pure (as_int c >= 0) **
      pure (as_int c >= 0 ==> as_int a == sum (as_int c)) **
      pure (b == (as_int c < as_int vn))))
  {
    ctr = ctr + 1;
    acc = acc + ctr;
  };
  return acc;
}

_include_pulse(
  let rec fib (n:nat) : nat =
    if n <= 1 then 1
    else fib (n - 1) + fib (n - 2)

  let rec fib_mono (n:nat) (m:nat { m <= n})
  : Lemma
    (ensures fib m <= fib n)
  = if n = m then ()
    else fib_mono (n - 1) m

)
_requires((_slprop) _inline_pulse(pure (as_int n > 0)))
_requires((_slprop) _inline_pulse(pure (as_int n > 0 ==> fib (as_int n) <= max_int32)))
_requires((_slprop) _inline_pulse(exists* v0 v1. (cur |-> v0) ** (prev |-> v1)))
_ensures((_slprop) _inline_pulse(exists* v0 v1.
  (cur |-> v0) **
  (prev |-> v1) **
  pure (as_int n > 0 ==> as_int v0 == fib (as_int n)) **
  pure (as_int n > 0 ==> as_int v1 == fib (as_int n - 1))))
void fib_rec (int n, int *cur, int *prev)
{
  if (n == 1)
  {
    *cur = 1;
    *prev = 1;
  }
  else
  {
    _assert((_slprop) _inline_pulse(with vn. assert n |-> vn));
    _assert((_slprop) _inline_pulse(fib_mono(as_int vn) (as_int vn - 1)));
    fib_rec(n - 1, cur, prev);
    int tmp = *cur;
    *cur = *cur + *prev;
    *prev = tmp;
  }
}

_requires((_slprop) _inline_pulse(pure (as_int n > 0)))
_requires((_slprop) _inline_pulse(pure (as_int n > 0 ==> fib (as_int n) <= max_int32)))
_ensures((_slprop) _inline_pulse(pure (as_int n > 0 ==> as_int i == fib (as_int n))))
int call_fib_rec (int n)
{
  int cur = 0; int prev = 0;
  fib_rec(n, &cur, &prev);
  return cur;
}
