module Issue24_test2

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



[@@expect_failure]
fn refs_are_scoped ()
returns s:ref int32
ensures 
s |-> 0l
{
let mut s : Int32.t = 0l;
(s);
}
