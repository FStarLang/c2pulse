#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define REQUIRES(s) [[clang::pulse("requires:"#s)]]
#define ENSURES(s) [[clang::pulse("ensures:"#s)]]
#define ISARRAY(len) [[clang::pulse("array:"#len)]]
#define INVARIANTS(...) [[clang::pulse("invariants:"#__VA_ARGS__)]]
#define LEMMA(l) [[clang::pulse("lemma:"#l)]]

REQUIRES(exists* s.arr |-> s)
REQUIRES(pure (length arr == SizeT.v len))
ENSURES(exists* s.arr |-> s)
void reverse(ISARRAY(len) uint32_t *arr, size_t len) {
  size_t i = 0;
  while (i < len / 2)
    INVARIANTS( invariant c. ,
                exists* vi. (i |->vi) ** (exists* s.arr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div len 2sz)) 
              ) 
        {
                size_t j = len - 1 - i;  
                LEMMA(pts_to_len arr);
                uint32_t tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
                i = i + 1;
        }
}