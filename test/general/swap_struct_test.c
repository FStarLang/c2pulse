#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;


REQUIRES(emp)
RETURNS(x:ref u32_pair_struct)
ENSURES(freeable x)
ENSURES((u32_pair_struct_pred x { first = 0ul; second = 1ul }))
u32_pair_struct* new_u32_pair_struct ()
{
  u32_pair_struct* x = ( u32_pair_struct* )malloc(sizeof(u32_pair_struct));
  x->first = 0ul;
  x->second = 1ul;
  return x;
}


ERASED_ARG(#s : u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES(u32_pair_struct_pred x ({first = s.second; second = s.first}))
void swap_fields(u32_pair_struct *x) {
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
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

ERASED_ARG(#s : u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES(u32_pair_struct_pred x ({first = s.second; second = s.first}))
void swap_fields_alt(u32_pair_struct *x) { 
  swap_refs(&x->first, &x->second); 
}

int main() {
  u32_pair_struct *x = new_u32_pair_struct();
  swap_fields(x);
  swap_fields_alt(x);
  LEMMA(with vx. assert x |-> vx);
  ASSERT(u32_pair_struct_pred vx {first = 0ul; second = 1ul});
  free(x);
  return EXIT_SUCCESS;
}
