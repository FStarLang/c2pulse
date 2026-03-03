#include "c2pulse.h"
#include <stdint.h>
#include <stdlib.h>

void reverse(_array uint32_t *arr, size_t len)
  _preserves(arr._length == len)
{
  size_t i = 0;
  while (i < len / 2)
    _invariant(_live(i))
    _invariant(_live(*arr))
    _invariant(arr._length == len)
    _invariant(i <= len / 2)
  {
    size_t j = len - 1 - i;  
    uint32_t tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
    i = i + 1;
  }
}