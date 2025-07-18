module Issue24_test

#lang-pulse

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn refs_are_scoped ()
returns s:ref int32
ensures s |-> 0l
{
let s0 : Int32.t = 0l;
let mut s : Int32.t = s0;
(s);
}
