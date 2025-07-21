module Issue9_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn refs_are_scoped ()
returns s:ref Pulse.Lib.C.Int32.int32
{
let mut s : Int32.t = 0l;
(s);
}
