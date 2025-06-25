// RUN: %c2pulse %s 
// RUN: cat %p/Reverse_test/Module_0.fst | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Reverse_test/Module_0.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include "../../test-transpiler/c/pulse_macros.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

// C2PULSE: fn reverse
// C2PULSE: (arr : array UInt32.t)
// C2PULSE: (len : SizeT.t)
// C2PULSE: requires exists* s.arr |-> s
// C2PULSE: requires pure (length arr == SizeT.v len)
// C2PULSE: ensures exists* s.arr |-> s
// C2PULSE: {
// C2PULSE: let mut i = 0sz;
// C2PULSE: while((SizeT.lt (! i) (SizeT.div len 2sz));
// C2PULSE: )
// C2PULSE: invariant c. 
// C2PULSE:  exists* vi. (i |->vi) ** (exists* s.arr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div len 2sz))
// C2PULSE: {
// C2PULSE: let j = (SizeT.sub (SizeT.sub len 1sz) (! i));
// C2PULSE: pts_to_len arr;
// C2PULSE: let tmp = (op_Array_Access arr (! i));
// C2PULSE: arr.((! i)) <- (op_Array_Access arr j);
// C2PULSE: arr.(j) <- tmp;
// C2PULSE: i := (SizeT.add (! i) 1sz);
// C2PULSE: }}

// PULSE: All verification conditions discharged successfully