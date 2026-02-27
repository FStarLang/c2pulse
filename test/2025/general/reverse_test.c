#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(exists* s.arr |-> s))
_requires((_slprop) _inline_pulse(pure (length arr == SizeT.v len)))
_ensures((_slprop) _inline_pulse(exists* s.arr |-> s))
void reverse(uint32_t *arr, size_t len) {
  size_t i = 0;
  _assert((_slprop) _inline_pulse(with varr. assert arr |-> varr));
  _assert((_slprop) _inline_pulse(with vlen. assert len |-> vlen));
  while (i < len / 2)
        _invariant((_slprop) _inline_pulse((arr |-> varr) **
                (len |-> vlen) ** (
                exists* vi. (i |->vi) ** (exists* s. varr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div vlen 2sz)) 
              )))
        {
                size_t j = len - 1 - i;  
                _assert((_slprop) _inline_pulse(pts_to_len varr));
                uint32_t tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
                i = i + 1;
        }
}
