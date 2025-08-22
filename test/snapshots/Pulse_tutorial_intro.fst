module Pulse_tutorial_intro

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn incr
(x : ( ref Int32.t) )
(#i:erased _)
requires 
x |-> i
requires 
pure (fits (+) (as_int i) 1)
ensures 
exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)
{
let mut x : (ref Int32.t) = x;
(! x) := (Int32.add (! (! x)) 1l);
}

fn incr_frame
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#i:_)
(#j:_)
requires 
x |-> i
requires 
y |-> j
requires 
pure (fits (+) (as_int i) 1)
ensures 
exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)
ensures 
y |-> j
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
(incr (! x));
}
