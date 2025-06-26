#include <stdint.h>
#include <stdlib.h>
#include "pulse_macros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;


RETURNS(x)
ENSURES (exists* (s:u32_pair_struct_spec). Box.box u32_pair_struct_pred x s ** pure (s == {first = 0ul; second = 0ul}))
u32_pair_struct* new_u32_pair_struct ()
{
  u32_pair_struct* x = ( u32_pair_struct* )malloc(sizeof(u32_pair_struct));
  x->first = 0ul;
  x->second = 0ul;
  return x;
}
