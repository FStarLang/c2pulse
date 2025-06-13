#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define REQUIRES(s) [[clang::pulse("requires:" #s)]]
#define ENSURES(s) [[clang::pulse("ensures:" #s)]]
#define ISARRAY(arr, len) [[clang::pulse("array:" #arr ":" #len)]]
#define INVARIANTS(...) [[clang::pulse("invariants:" #__VA_ARGS__)]]
#define LEMMA(l) [[clang::pulse("lemma:" #l)]]

REQUIRES(exists *s.arr |->s)
REQUIRES(pure(length arr == SizeT.v len))
ENSURES(exists *s.arr |->s)
ISARRAY(arr, len)
void reverse(uint32_t *arr, size_t len) {
  size_t i = 0;
  while (i < len / 2)
    INVARIANTS(invariant c, exists * vi.(i |->vi) * *(exists * s.arr |->s) *
                                *pure(c == (vi `SizeT.lt` SizeT.div len 2sz))) {
      size_t __anf_tmp1 = len - 1;
      size_t __anf_tmp2 = __anf_tmp1 - i;
      size_t j = __anf_tmp2;
      LEMMA(pts_to_len arr)
      uint32_t tmp = arr[i];
      uint32_t __anf_tmp4 = arr[j];
      arr[i] = __anf_tmp4;
      arr[j] = tmp;
      i = i + 1;
    }
}

int main() { return 0; }