#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

REQUIRES(exists* v. (x |-> v) ** pure (as_int v >= 0))
ENSURES(x |-> 0l)
void count_down (int *x)
{
  int keep_going = 1;
  LEMMA (with vx. assert (x |-> vx));
  while (keep_going == 1)
  INVARIANTS(invariant b.
    exists* k v.
      (keep_going |-> k) **
      (x |-> vx) ** (* tedious *)
      (vx |-> v) **
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
INCLUDE ( 
  module U32 = Pulse.Lib.C.UInt32
)

REQUIRES(pure (U32.fits ( * ) (U32.as_int x) (U32.as_int y)))
RETURNS(i:_)
ENSURES(pure (U32.as_int i == U32.as_int x * U32.as_int y))
uint32_t multiply_by_repeated_addition (uint32_t x, uint32_t y)
{
  uint32_t ctr = 0;
  uint32_t acc = 0;
  LEMMA(with vx vy. assert (x |-> vx) ** (y |-> vy));
  while (ctr < x)
  INVARIANTS(invariant 
    exists* c a.
      (ctr |-> c) **
      (acc |-> a) **
      pure (U32.as_int c <= U32.as_int vx) **
      pure (U32.as_int a == U32.as_int c * U32.as_int vy)
  )
  {
    ctr = ctr + 1;
    acc = acc + y;
  }
  return acc;
}

INCLUDE (
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

REQUIRES(pure (as_int n >= 0))
REQUIRES(pure (fits (+) (as_int n) 1))
REQUIRES(pure (fits ( * ) (as_int n) (as_int n + 1)))
RETURNS(i:int32)
ENSURES(pure (as_int i == (as_int n * (as_int n + 1)) / 2))
int isum (int n)
{
  int acc = 0;
  int ctr = 0;
  LEMMA(sum_lemma(as_int !n));
  LEMMA(with vn. assert (n |-> vn));
  while (ctr < n)
  INVARIANTS(invariant b.
    exists* c a.
      (n |-> vn) ** (* tedious *)
      (ctr |-> c) **
      (acc |-> a) **
      pure (as_int c <= as_int vn) **
      pure (as_int c >= 0) **
      pure (as_int c >= 0 ==> as_int a == sum (as_int c)) **
      pure (b == (as_int c < as_int vn))
  )
  {
    ctr = ctr + 1;
    acc = acc + ctr;
  };
  return acc;
}

INCLUDE (
  let rec fib (n:nat) : nat =
    if n <= 1 then 1
    else fib (n - 1) + fib (n - 2)

  let rec fib_mono (n:nat) (m:nat { m <= n})
  : Lemma
    (ensures fib m <= fib n)
  = if n = m then ()
    else fib_mono (n - 1) m

)
REQUIRES(pure (as_int n > 0))
REQUIRES(pure (as_int n > 0 ==> fib (as_int n) <= max_int32))
REQUIRES(exists* v0 v1. (cur |-> v0) ** (prev |-> v1))
ENSURES(exists* v0 v1.
  (cur |-> v0) **
  (prev |-> v1) **
  pure (as_int n > 0 ==> as_int v0 == fib (as_int n)) **
  pure (as_int n > 0 ==> as_int v1 == fib (as_int n - 1)))
void fib_rec (int n, int *cur, int *prev)
{
  if (n == 1)
  {
    *cur = 1;
    *prev = 1;
  }
  else
  {
    LEMMA (with vn. assert n |-> vn);
    LEMMA(fib_mono(as_int vn) (as_int vn - 1));
    fib_rec(n - 1, cur, prev);
    int tmp = *cur;
    *cur = *cur + *prev;
    *prev = tmp;
  }
}

REQUIRES(pure (as_int n > 0))
REQUIRES(pure (as_int n > 0 ==> fib (as_int n) <= max_int32))
RETURNS(i:int32)
ENSURES(pure (as_int n > 0 ==> as_int i == fib (as_int n)))
int call_fib_rec (int n)
{
  int cur = 0; int prev = 0;
  fib_rec(n, &cur, &prev);
  return cur;
}
