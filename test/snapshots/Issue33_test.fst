module Issue33_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn count_down
(x : ref UInt32.t)
requires exists* v. x |-> v
returns UInt32.t
ensures exists* v. x |-> v
{
let mut x : (ref UInt32.t) = x;
(int32_to_uint32 ((bool_to_int32 (UInt32.eq (! (! x)) (int32_to_uint32 0l)))));
}

fn decr
(x : ref UInt32.t)
requires exists* v. x |-> v
returns b:FStar.UInt32.t
ensures exists* v. x |-> v
{
let mut x : (ref UInt32.t) = x;
((UInt32.sub (! (! x)) (int32_to_uint32 0l)));
}
