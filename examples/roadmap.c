#include "pal.h"
#include <stdint.h>
#include <stdlib.h>

void swap(uint32_t *x, uint32_t *y)
  _ensures(*y == _old(*x) && *x == _old(*y))
{
  int tmp = *y;
  *y = *x;
  *x = tmp;
}

typedef struct {
  uint32_t first;
  uint32_t second;
} u32_pair;

_allocated
typedef u32_pair* freeable_u32_pair_ptr;

freeable_u32_pair_ptr new_u32_pair()
  _ensures(return->first == 0u)
  _ensures(return->second == 1u)
{
  u32_pair *x = (u32_pair *)malloc(sizeof(u32_pair));
  _ghost_stmt(struct_u32_pair_anon_1__aux_raw_unfold_uninit $(x));
  x->first = 0ul;
  x->second = 1ul;
  return x;
}

void swap_fields(u32_pair *x)
  _ensures(x->first == _old(x->second))
  _ensures(x->second == _old(x->first))
{
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}

_ensures(x->first == _old(x->second) && x->second == _old(x->first))
void swap_fields_alt(u32_pair *x) { swap(&x->first, &x->second); }

int test_swaps()
  _ensures(return == EXIT_SUCCESS)
{
  u32_pair *x = new_u32_pair();
  swap_fields(x);
  swap_fields_alt(x);
  _assert(x->first == 0ul && x->second == 1ul);
  free(x);
  return EXIT_SUCCESS;
}

uint32_t multiply_by_repeated_addition(uint32_t x, uint32_t y)
  _requires((_specint) x * y <= UINT32_MAX)
  _ensures(return == x * y)
{
  uint32_t ctr = 0;
  uint32_t acc = 0;
  while (ctr < x)
    _invariant(_live(ctr) && _live(acc))
    _invariant(ctr <= x && acc == ctr * y)
  {
      ctr = ctr + 1;
      acc = acc + y;
  }
  return acc;
}