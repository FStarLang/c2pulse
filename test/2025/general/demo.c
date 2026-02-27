#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_include_pulse(open Pulse.Lib.C.UInt32)

_ensures(*x == _old(*y))
_ensures(*y == _old(*x))
void swap_refs(uint32_t *x, uint32_t *y) {
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair;

_ensures((_slprop) _inline_pulse(freeable x))
_ensures((_slprop) _inline_pulse(u32_pair_pred x ({ first = 0ul; second = 1ul })))
u32_pair* new_u32_pair ()
{
  u32_pair* x = (u32_pair*)malloc(sizeof(u32_pair));
  x->first = 0ul;
  x->second = 1ul;
  return x;
}

_requires((_slprop) _inline_pulse(u32_pair_pred x s))
_ensures((_slprop) _inline_pulse(u32_pair_pred x ({first=s.second; second=s.first})))
void swap_fields(u32_pair *x) {
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}

_requires((_slprop) _inline_pulse(u32_pair_pred x s))
_ensures((_slprop) _inline_pulse(u32_pair_pred x ({first = s.second; second = s.first})))
void swap_fields_alt(u32_pair *x) { 
  swap_refs(&x->first, &x->second); 
}

int test_swaps() {
  u32_pair *x = new_u32_pair();
  swap_fields(x);
  swap_fields_alt(x);
  _assert((_slprop) _inline_pulse(u32_pair_pred (!x) {first = 0ul; second = 1ul}));
  free(x);
  return EXIT_SUCCESS;
}

_requires((_slprop) _inline_pulse(pure (fits ( * ) (as_int x) (as_int y))))
_ensures((_slprop) _inline_pulse(pure (as_int i == as_int x * as_int y)))
uint32_t multiply_by_repeated_addition (uint32_t x, uint32_t y)
{
  uint32_t ctr = 0;
  uint32_t acc = 0;
  while (ctr < x)
      _invariant(_live(ctr))
    _invariant(_live(acc))
    _invariant((_slprop) _inline_pulse(pure (as_int !ctr <= as_int !x)))
    _invariant((_slprop) _inline_pulse(pure (as_int !acc == as_int !ctr * as_int !y)))
  {
    ctr = ctr + 1;
    acc = acc + y;
  }
  return acc;
}