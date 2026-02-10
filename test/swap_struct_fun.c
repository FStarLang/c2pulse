#include "c2pulse.h"

typedef struct {
  int a;
  int b;
} int_pair;

int_pair swap_functional(int_pair x)
{
  return (int_pair) { .a = x.b, .b = x.a };
}