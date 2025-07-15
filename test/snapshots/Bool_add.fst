module Bool_add

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(a : bool)
(b : bool)
returns Int32.t
{
(Int32.add (bool_to_int32 a) (bool_to_int32 b));
}
