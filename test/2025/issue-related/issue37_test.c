#include "../include/PulseMacros.h"

ERASED_ARG(#v: erased _)
REQUIRES(n |-> v)
ENSURES(n |-> 0l)
void set(int *n)
{ *n = 0; }

RETURNS(res:int32)
int call_fib_rec (int n)
{
  int cur; 
  set(&cur);
  return cur;
}