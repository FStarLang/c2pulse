module Issue16_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn incr
(r : ( ref Int32.t) )
(#vr:erased _)
requires 
r |-> vr
requires 
pure Pulse.Lib.C.Int32.(fits (+) (as_int vr) 1)
returns Int32.t
ensures 
exists* w. (r |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == as_int vr + 1)
{
let mut r : (ref Int32.t) = r;
(! r) := (Int32.add (! (! r)) 1l);
(! (! r));
}

fn one ()
requires 
emp
returns i:Pulse.Lib.C.Int32.int32
ensures 
pure Pulse.Lib.C.Int32.(as_int i == 1)
{
let mut i : Int32.t = 0l;
(incr (i));
(! i);
}
