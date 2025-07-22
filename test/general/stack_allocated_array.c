#include <stddef.h>
#include "../include/PulseMacros.h"

REQUIRES(pure (abs (Int32.v len) < 100))
REQUIRES(pure ((Int32.v len) > 0))
void foo(int len){
   int arr[10];
   int y = (len + 2) * 2; 
   int arr1[y];
   return;
} 
