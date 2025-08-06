module Issue55_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn compare
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : SizeT.t)
requires pure (length a2 == SizeT.v l)
requires pure (length a1 == SizeT.v l)
returns bool
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : SizeT.t = l;
let mut i : SizeT.t = (int32_to_sizet 0l);
while((int32_to_bool (if (int32_to_bool ((bool_to_int32 (SizeT.lt (! i) (! l)))))
then (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i))))
else 0l));
)
invariant c. emp
{
i := (SizeT.add (! i) (int32_to_sizet 1l));
};
(int32_to_bool ((bool_to_int32 (SizeT.eq (! i) (! l)))));
}
