module Bool_add

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo
(a : bool)
(b : bool)
returns r: Int32.t
ensures 
pure (v r == (if a then 1 else 0) + (if b then 1 else 0))
{
let mut a : bool = a;
let mut b : bool = b;
(Int32.add (bool_to_int32 (! a)) (bool_to_int32 (! b)));
}
