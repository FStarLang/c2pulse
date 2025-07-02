#include <stdint.h>
#include <stdlib.h>
#include "../../../include/PulseMacros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

typedef struct _u64_pair_struct {
  uint64_t first;
  u32_pair_struct *second;
} u64_pair_struct;


/*REQUIRES(emp)
RETURNS (x:Box.box u32_pair_struct)
ENSURES (exists* (s:u32_pair_struct_spec). u32_pair_struct_pred (Box.box_to_ref x) s ** pure (s == {first = 0ul; second = 1ul}))
u32_pair_struct* new_u32_pair_struct()
{
  u32_pair_struct* x = (u32_pair_struct*)malloc(sizeof(u32_pair_struct));
  x->first = 0;
  x->second = 1;
  return x;
}

ERASED_ARG(#s:u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES("exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))")
void swap_fields (u32_pair_struct* x)
{
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}

REQUIRES("x |-> 'x")
REQUIRES("y |-> 'y")
ENSURES("x |-> 'y")
ENSURES("y |-> 'x")
void swap_refs (uint32_t* x, uint32_t* y)
{
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}*/

// ERASED_ARG(#s:u32_pair_struct_spec)
// REQUIRES(u32_pair_struct_pred x s)
// ENSURES("exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first}))")
// void swap_fields_alt (u32_pair_struct* x)
// {
//   swap_refs(&x->first, &x->second);
// }

/*REQUIRES(emp)
ENSURES(emp)
int main ()
{
  HEAPALLOCATED()u32_pair_struct* x = new_u32_pair_struct();
  swap_fields (x);  //translate the heap allocated Box to a ref
  //swap_fields_alt (x); //translate the heap allocated Box to a ref
  //ASSERT (u32_pair_struct_pred x {first = 0ul; second = 1ul});
  free(x);
}*/
