module Pulse_tutorial_intro

#lang-pulse

open Pulse
open Pulse.Lib.C



fn incr
(x : ref Int32.t)
requires x |-> 'i
requires pure (fits (+) (as_int 'i) 1)
ensures exists* j. (x |-> j) ** pure (as_int 'i + 1 == as_int j)
{
x := (Int32.add (! x) 1l);
}

fn incr_explicit_i
(x : ref Int32.t)
(#i:_)
requires x |-> i
requires pure (fits (+) (as_int i) 1)
ensures exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)
{
x := (Int32.add (! x) 1l);
}

fn incr_frame
(x : ref Int32.t)
(y : ref Int32.t)
(#i:_)
(#j:_)
requires x |-> i
requires y |-> j
requires pure (fits (+) (as_int i) 1)
ensures exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)
ensures y |-> j
{
(incr x);
}
