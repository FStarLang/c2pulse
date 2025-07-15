module Simple_reference_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn refs_are_scoped ()
returns s:Pulse.Lib.C.Int32.int32
ensures pure Pulse.Lib.C.Int32.(as_int s == 0)
{
let mut s : Int32.t = 0l;
((! s));
}
