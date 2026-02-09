#include "c2pulse.h"

typedef struct {
  int a;
  int b;
} int_pair;

void swap_inplace(int_pair *x)
  _ensures(x->a == _old(x->b))
  _ensures(x->b == _old(x->a))
{
  int tmp = x->a;
  x->a = x->b;
  x->b = tmp;
}