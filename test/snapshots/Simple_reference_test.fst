module Simple_reference_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn refs_are_scoped ()
returns s:ref Pulse.Lib.C.Int32.int32
{
let mut s : Int32.t = 0l;
(s);
}
