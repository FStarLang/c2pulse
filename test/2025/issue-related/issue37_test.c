#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(n |-> v))
_ensures((_slprop) _inline_pulse(n |-> 0l))
void set(int *n)
{ *n = 0; }

int call_fib_rec (int n)
{
  int cur; 
  set(&cur);
  return cur;
}