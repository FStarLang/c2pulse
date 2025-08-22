module Issue2_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn value_of_explicit
(r : ( ref Int32.t) )
(#w:_)
requires 
r |-> w
returns v:Int32.t
ensures 
r |-> w
ensures 
pure (v == w)
{
let mut r : (ref Int32.t) = r;
(! (! r));
}
