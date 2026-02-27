#include "../c2pulse.h"

_requires((_slprop) _inline_pulse((r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)))
_ensures((_slprop) _inline_pulse((r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)))
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}