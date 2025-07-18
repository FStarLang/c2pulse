module Issue54_test_3

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test_ampamp ()
returns Int32.t
{
let mut a: Int32.t = witness #_ #_;
let mut b: Int32.t = witness #_ #_;
(bool_to_int32 (op_AmpAmp (int32_to_bool (! a)) (int32_to_bool (! b))));
}

fn test_barbar ()
returns Int32.t
{
let mut a: Int32.t = witness #_ #_;
let mut b: Int32.t = witness #_ #_;
(bool_to_int32 (op_BarBar (int32_to_bool (! a)) (int32_to_bool (! b))));
}

fn compare_elt
(a1 : array Int32.t)
(a2 : array Int32.t)
(len : SizeT.t)
requires pure (length a2 == SizeT.v len)
requires pure (length a1 == SizeT.v len)
requires exists* v. (a1 |-> v)
requires exists* v. (a2 |-> v)
returns _Bool
ensures exists* v. (a1 |-> v)
ensures exists* v. (a2 |-> v)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut len : SizeT.t = len;
let i0 : SizeT.t = (int32_to_sizet 0l);
let mut i : SizeT.t = i0;
if((int32_to_bool (bool_to_int32 (SizeT.lt (! i) (! len)))))
{
pts_to_len !a1;
pts_to_len !a2;
(int32_to_bool ((bool_to_int32 (op_AmpAmp (int32_to_bool ((bool_to_int32 (SizeT.lt (! i) (! len))))) (int32_to_bool (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i)))))))));
}
else
{
(int32_to_bool 0l);
};
}
