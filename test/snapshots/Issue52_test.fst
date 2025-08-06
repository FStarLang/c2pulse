module Issue52_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn fake_arr
(x : array Int32.t)
requires pure (length x == SizeT.v 5sz)
requires exists* v. (x |-> v)
returns Int32.t
ensures exists* v. (x |-> v)
{
let mut x : (array Int32.t) = x;
pts_to_len !x;
(op_Array_Access (! x) (int32_to_sizet 0l));
}
