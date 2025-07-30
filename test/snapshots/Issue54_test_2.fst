module Issue54_test_2

#lang-pulse

open Pulse
open Pulse.Lib.C



fn add ()
returns Int32.t
{
let mut x : Int32.t = 1l;
let mut y : Int32.t = 0l;
(Int32.add (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! x))))))) (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! y))))))));
}

fn compare_elt
(a1 : array Int32.t)
(a2 : array Int32.t)
(i : SizeT.t)
(len : SizeT.t)
requires pure (length a1 == SizeT.v len)
requires pure (length a2 == SizeT.v len)
requires exists* v. (a1 |-> v)
requires exists* v. (a2 |-> v)
returns Int32.t
ensures exists* v. (a1 |-> v)
ensures exists* v. (a2 |-> v)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut i : SizeT.t = i;
let mut len : SizeT.t = len;
if((int32_to_bool (bool_to_int32 (SizeT.lt (! i) (! len)))))
{
pts_to_len !a1;
pts_to_len !a2;
((bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i)))));
}
else
{
(int64_to_int32 0L);
};
}
