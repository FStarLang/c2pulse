
// RUN: %c2pulse %s 2>&1 | %{FILECHECK} %s

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

// CHECK: The pulse function Name is: reverse

// CHECK: Print Program:

// CHECK: fn reverse
// CHECK-NEXT: (arr : array UInt32.t)
// CHECK-NEXT: (len : SizeT.t)
// CHECK-NEXT: requires exists* s.arr |-> s
// CHECK-NEXT: requires pure (length arr == SizeT.v len)
// CHECK-NEXT: ensures exists* s.arr |-> s
// CHECK-NEXT: {
// CHECK-NEXT: let mut i = 0sz;
// CHECK-NEXT: while((SizeT.lt (! i) (SizeT.div len 2sz));
// CHECK-NEXT: )
// CHECK-NEXT: invariant c. 
// CHECK-NEXT:  exists* vi. (i |->vi) ** (exists* s.arr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div len 2sz))
// CHECK-NEXT: {
// CHECK-NEXT: let j = (SizeT.sub (SizeT.sub len 1sz) (! i));
// CHECK-NEXT: pts_to_len arr;
// CHECK-NEXT: let tmp = (op_Array_Access arr (! i));
// CHECK-NEXT: arr.((! i)) <- (op_Array_Access arr j);
// CHECK-NEXT: arr.(j) <- tmp;
// CHECK-NEXT: i := (SizeT.add (! i) 1sz);
// CHECK: }}


// CHECK: Success: Code transformed and syntax validated.