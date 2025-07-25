module Issue55_test

#lang-pulse

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn compare
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : UInt64.t)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : UInt64.t = l;
let mut i : UInt64.t = (int32_to_uint64 0l);
while((int32_to_bool (if (int32_to_bool ((bool_to_int32 (UInt64.lt (! i) (! l)))))
then (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (uint64_to_sizet (! i))) (op_Array_Access (! a2) (uint64_to_sizet (! i)))))
else 0l));
)
invariant c. emp
{
i := (UInt64.add (! i) (int32_to_uint64 1l));
};
(int32_to_bool ((bool_to_int32 (UInt64.eq (! i) (! l)))));
}
