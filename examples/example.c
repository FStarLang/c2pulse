#include "c2pulse.h"
#include <stdint.h>

void swap(int *x, int *y)
    _ensures(*y == _old(*x) && *x == _old(*y))
{
    int tmp = *y;
    *y = *x;
    *x = tmp;
}

int max(int a, int b)
    _ensures(a <= return && b <= return)
{
    if (a <= b) {
        return b;
    } else {
        return a;
    }
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