module Reverse_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn reverse
(arr : array UInt32.t)
(len : UInt64.t)
requires exists* s.arr |-> s
requires pure (length arr == UInt64.v len)
ensures exists* s.arr |-> s
{
let mut arr : (array UInt32.t) = arr;
let mut len : UInt64.t = len;
let mut i : UInt64.t = (int32_to_uint64 0l);
with varr. assert arr |-> varr;
with vlen. assert len |-> vlen;
while((int32_to_bool (bool_to_int32 (UInt64.lt (! i) (UInt64.div (! len) (int32_to_uint64 2l)))));
)
invariant c. (arr |-> varr) ** (len |-> vlen) ** ( exists* vi. (i |->vi) ** (exists* s. varr |->s) ** pure (c == (vi `UInt64.lt` UInt64.div vlen 2UL)) )
{
let mut j : UInt64.t = (UInt64.sub (UInt64.sub (! len) (int32_to_uint64 1l)) (! i));
pts_to_len varr;
let mut tmp : UInt32.t = (op_Array_Access (! arr) (uint64_to_sizet (! i)));
(! arr).(uint64_to_sizet (! i)) <- (op_Array_Access (! arr) (uint64_to_sizet (! j)));
(! arr).(uint64_to_sizet (! j)) <- (! tmp);
i := (UInt64.add (! i) (int32_to_uint64 1l));
};
}
