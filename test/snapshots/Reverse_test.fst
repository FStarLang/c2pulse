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
let mut arr : (array UInt32.t) = arr;
let mut len : SizeT.t = len;
let mut i : SizeT.t = (int32_to_sizet 0l);
with varr. assert arr |-> varr;
with vlen. assert len |-> vlen;
while((int32_to_bool (bool_to_int32 (SizeT.lt (! i) (SizeT.div (! len) (int32_to_sizet 2l)))));
)
invariant c. (arr |-> varr) ** (len |-> vlen) ** ( exists* vi. (i |->vi) ** (exists* s. varr |->s) ** pure (c == (vi `SizeT.lt` SizeT.div vlen 2sz)) )
{
let mut j : SizeT.t = (SizeT.sub (SizeT.sub (! len) (int32_to_sizet 1l)) (! i));
pts_to_len varr;
let mut tmp : UInt32.t = (op_Array_Access (! arr) (! i));
(! arr).((! i)) <- (op_Array_Access (! arr) (! j));
(! arr).((! j)) <- (! tmp);
i := (SizeT.add (! i) (int32_to_sizet 1l));
};}
