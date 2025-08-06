module Bool_add

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo
(a : bool)
(b : bool)
returns Int32.t
{
let mut a : bool = a;
let mut b : bool = b;
(Int32.add (bool_to_int32 (! a)) (bool_to_int32 (! b)));
}
