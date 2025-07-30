#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

REQUIRES(exists* s.arr |-> s)
REQUIRES(pure (length arr == SizeT.v len))
ENSURES(exists* s.arr |-> s)
void reverse(ISARRAY(len) uint32_t *arr, size_t len) {
  size_t i = 0;
  LEMMA(with varr. assert arr |-> varr);
  LEMMA(with vlen. assert len |-> vlen);
  while (i < len / 2)
    INVARIANTS( invariant c.
                (arr |-> varr) **
                (len |-> vlen) ** (
                exists* vi. (i |->vi) ** (exists* s. varr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div vlen 2sz)) 
              ) )
        {
                size_t j = len - 1 - i;  
                LEMMA(pts_to_len varr);
                uint32_t tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
                i = i + 1;
        }
}
