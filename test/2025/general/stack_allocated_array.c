#include <stddef.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(pure (abs (Int32.v len) < 100)))
_requires((_slprop) _inline_pulse(pure ((Int32.v len) > 0)))
void foo(int len){
   int arr[10];
   int y = (len + 2) * 2; 
   int arr1[y];
   return;
} 
