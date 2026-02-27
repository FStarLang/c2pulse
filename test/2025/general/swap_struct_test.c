#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

_ensures((_slprop) _inline_pulse(freeable x))
_ensures((_slprop) _inline_pulse((u32_pair_struct_pred x { first = 0ul; second = 1ul })))
u32_pair_struct* new_u32_pair_struct ()
{
  u32_pair_struct* x = ( u32_pair_struct* )malloc(sizeof(u32_pair_struct));
  x->first = 0ul;
  x->second = 1ul;
  return x;
}

_requires((_slprop) _inline_pulse(u32_pair_struct_pred x s))
_ensures((_slprop) _inline_pulse(u32_pair_struct_pred x ({first = s.second; second = s.first})))
void swap_fields(u32_pair_struct *x) {
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}

_ensures((_slprop) _inline_pulse(rewrites_to (!x) (old(!y))))
_ensures((_slprop) _inline_pulse(rewrites_to (!y) (old(!x))))
void swap_refs(uint32_t *x, uint32_t *y) {
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

_requires((_slprop) _inline_pulse(u32_pair_struct_pred x s))
_ensures((_slprop) _inline_pulse(u32_pair_struct_pred x ({first = s.second; second = s.first})))
void swap_fields_alt(u32_pair_struct *x) { 
  swap_refs(&x->first, &x->second); 
}

int main() {
  u32_pair_struct *x = new_u32_pair_struct();
  swap_fields(x);
  swap_fields_alt(x);
  _assert((_slprop) _inline_pulse(with vx. assert x |-> vx));
  _assert((_slprop) _inline_pulse(u32_pair_struct_pred vx {first = 0ul; second = 1ul}));
  free(x);
  return EXIT_SUCCESS;
}
