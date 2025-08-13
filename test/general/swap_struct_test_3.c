#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair;


RETURNS(x:_)
ENSURES(freeable x)
ENSURES(u32_pair_pred x ({ first = 0ul; second = 1ul }))
u32_pair* new_u32_pair ()
{
  u32_pair* x = (u32_pair* )malloc(sizeof(u32_pair));
  x->first = 0ul;
  x->second = 1ul;
  return x;
}


ERASED_ARG(#s : _)
REQUIRES(u32_pair_pred x s)
ENSURES(u32_pair_pred x ({first = s.second; second = s.first}))
void swap_fields(u32_pair *x) {
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}


ERASED_ARG(#vx #vy:erased _)
//ERASED_ARG(vx:_)
REQUIRES(x |-> vx)
//ERASED_ARG(vy:_)
REQUIRES(y |-> vy)
ENSURES(x |-> vy)
ENSURES(y |-> vx)
void swap_refs(uint32_t *x, uint32_t *y) {
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}


ERASED_ARG(#s : erased _)
REQUIRES(u32_pair_pred x s)
ENSURES(u32_pair_pred x ({first = s.second; second = s.first}))
void swap_fields_alt(u32_pair *x) { 
  swap_refs(&x->first, &x->second); 
}


int main() {
  u32_pair *x = new_u32_pair();
  swap_fields(x);
  swap_fields_alt(x);
  LEMMA(with vx. assert ((x |-> vx) ** u32_pair_pred vx {first = 0ul; second = 1ul}));
  free( x);
  return EXIT_SUCCESS;
}