module Reverse_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn reverse
(arr : array UInt32.t)
(len : SizeT.t)
requires pure (length arr == SizeT.v len)
requires exists* s.arr |-> s
requires pure (length arr == SizeT.v len)
ensures exists* s.arr |-> s
{
let mut i = 0sz;
while((SizeT.lt (! i) (SizeT.div len 2sz));
)
invariant c. 
 exists* vi. (i |->vi) ** (exists* s.arr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div len 2sz))
{
let j = (SizeT.sub (SizeT.sub len 1sz) (! i));
pts_to_len arr;
let tmp = (op_Array_Access arr (! i));
arr.((! i)) <- (op_Array_Access arr j);
arr.(j) <- tmp;
i := (SizeT.add (! i) 1sz);
};}
