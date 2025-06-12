#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define __PULSE_VERIFICATION 1

#if __PULSE_VERIFICATION
 void __pulseproofterm(const char *lemma);
 void __pulsewhileinvariant(const char* invariant, const char* lemma);
 #define PROOFTERM(l) __pulseproofterm(#l);
 #define WHILEINVARIANT(i, l) __pulsewhileinvariant(#i, #l);
 #define META_WHILE(condition, ...) \
     __VA_ARGS__ \
     while (condition)    
#else
 #define PROOFTERM(s)
 #define WHILEINVARIANT(s, v)
 #define META_WHILE(condition, meta) \
    while (condition)
#endif

///@requires exists* s. arr |-> s
///@requires pure (length arr == SizeT.v len)
///@ensures  exists* s. arr |-> s
void reverse(uint32_t *arr, size_t len)
{
  size_t i = 0;
  META_WHILE(i < len / 2, 
   WHILEINVARIANT(invariant b., 
                    exists* vi.
                    (i |-> vi) **
                    (exists* s. arr |-> s) **
                    pure (b == (vi `SizeT.lt` SizeT.div len 2sz))
                    )
                    ) 
  {
    size_t __anf_tmp1 = len - 1;
    size_t __anf_tmp2 = __anf_tmp1 - i;
    size_t j = __anf_tmp2;
    PROOFTERM(pts_to_len arr)
    uint32_t tmp = arr[i];
    uint32_t __anf_tmp4 = arr[j];
    arr[i] = __anf_tmp4;
    arr[j] = tmp;
    i = i + 1;
  }
}

// int main () {
//   return 0;
// }



  //  WHILEINVARIANT(invariant c, exists* vi.
  //                   (i |-> vi) **
  //                   (exists* s. arr |-> s) **
  //                   pure (c == (vi `SizeT.lt` SizeT.div len 2sz)))