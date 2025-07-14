module Issue52_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn fake_arr
(x : array Int32.t)
requires pure (length x == SizeT.v 5sz)
requires exists* v. (x |-> v)
returns Int32.t
ensures exists* v. (x |-> v)
{
pts_to_len x;
(op_Array_Access x 0sz);
}
