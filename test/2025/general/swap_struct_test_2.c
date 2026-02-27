#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} _u32_pair_struct;

_ensures((_slprop) _inline_pulse(freeable x))
_ensures((_slprop) _inline_pulse((_u32_pair_struct_pred x { first = 0ul; second = 1ul })))
struct _u32_pair_struct* new_u32_pair_struct ()
{
  struct _u32_pair_struct* x = ( struct _u32_pair_struct* )malloc(sizeof(struct _u32_pair_struct));
  // _assert((_slprop) _inline_pulse(_u32_pair_struct_explode !x));
  x->first = 0ul;
  x->second = 1ul;
  // _assert((_slprop) _inline_pulse(_u32_pair_struct_recover !x));
  return x;
}

_requires((_slprop) _inline_pulse(_u32_pair_struct_pred x s))
_ensures((_slprop) _inline_pulse(exists* (s2:_u32_pair_struct_spec). _u32_pair_struct_pred x s2 ** pure (s2 == ({first = s.second; second = s.first}))))
void swap_fields(struct _u32_pair_struct *x) {
  _assert((_slprop) _inline_pulse(_u32_pair_struct_explode !x));
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
  _assert((_slprop) _inline_pulse(_u32_pair_struct_recover !x));
}

_ensures((_slprop) _inline_pulse(rewrites_to (!x) (old(!y))))
_ensures((_slprop) _inline_pulse(rewrites_to (!y) (old(!x))))
void swap_refs(uint32_t *x, uint32_t *y) {
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

_requires((_slprop) _inline_pulse(_u32_pair_struct_pred x s))
_ensures((_slprop) _inline_pulse(exists* (s2:_u32_pair_struct_spec). _u32_pair_struct_pred x s2 ** pure (s2 == {first = s.second; second = s.first})))
void swap_fields_alt(struct _u32_pair_struct *x) { 
  _assert((_slprop) _inline_pulse(_u32_pair_struct_explode !x));
  swap_refs(&x->first, &x->second); 
  _assert((_slprop) _inline_pulse(_u32_pair_struct_recover !x));
}

int main() {
  struct _u32_pair_struct *x = new_u32_pair_struct();
  swap_fields(x);
  swap_fields_alt(x);
  _assert((_slprop) _inline_pulse(with vx. assert ((x |-> vx) ** _u32_pair_struct_pred vx {first = 0ul; second = 1ul})));
  free( x);
  return EXIT_SUCCESS;
}
