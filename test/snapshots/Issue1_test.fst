module Issue1_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn incr
(x : ( ref Int32.t) )
requires x |-> 'i
requires pure FStar.Int32.(fits (v 'i + 1))
ensures exists* j. (x |-> j) ** pure FStar.Int32.((v j <: int) == v 'i + 1)
{
let mut x : (ref Int32.t) = x;
(! x) := (Int32.add (! (! x)) 1l);
}

fn incr_frame
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#i:_)
(#j:_)
requires x |-> i
requires y |-> j
requires pure FStar.Int32.(fits (v i + 1))
ensures exists* k. (x |-> k) ** pure FStar.Int32.(v i + 1 == v k)
ensures y |-> j
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
(incr (! x));
}
