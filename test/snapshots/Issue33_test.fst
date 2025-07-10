module Issue33_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn count_down
(x : ref UInt32.t)
requires exists* v. x |-> v
returns b:bool
ensures exists* v. x |-> v
{
((UInt32.eq (! x) 0ul));
}

fn decr
(x : ref UInt32.t)
requires exists* v. x |-> v
returns b:FStar.UInt32.t
ensures exists* v. x |-> v
{
((UInt32.sub (! x) 0ul));
}
