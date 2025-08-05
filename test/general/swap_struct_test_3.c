#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
};


REQUIRES(emp)
RETURNS(x:ref _u32_pair_struct)
ENSURES(freeable x)
ENSURES((_u32_pair_struct_pred x { first = 0ul; second = 1ul }))
struct _u32_pair_struct* new_u32_pair_struct ()
{
  struct _u32_pair_struct* x = ( struct _u32_pair_struct* )malloc(sizeof(struct _u32_pair_struct));
  LEMMA(_u32_pair_struct_explode !x);
  x->first = 0ul;
  x->second = 1ul;
  LEMMA(_u32_pair_struct_recover !x);
  return x;
}

ERASED_ARG(#s : _u32_pair_struct_spec)
REQUIRES(_u32_pair_struct_pred x s)
ENSURES("exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))")
void swap_fields(struct _u32_pair_struct *x) {
  LEMMA(_u32_pair_struct_explode !x);
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
  LEMMA(_u32_pair_struct_recover !x);
}

REQUIRES("x |-> 'x")
REQUIRES("y |-> 'y")
ENSURES("x |-> 'y")
ENSURES("y |-> 'x")
void swap_refs(uint32_t *x, uint32_t *y) {
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

ERASED_ARG(#s : _u32_pair_struct_spec)
REQUIRES(_u32_pair_struct_pred x s)
ENSURES("exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})")
void swap_fields_alt(struct _u32_pair_struct *x) { 
  LEMMA(_u32_pair_struct_explode !x);
  swap_refs(&x->first, &x->second); 
  LEMMA(_u32_pair_struct_recover !x);
}

int main() {
  struct _u32_pair_struct *x = new_u32_pair_struct();
  swap_fields(x);
  swap_fields_alt(x);
  LEMMA(with vx. assert ((x |-> vx) ** _u32_pair_struct_pred vx {first = 0ul; second = 1ul}));
  free( x);
  return EXIT_SUCCESS;
}
