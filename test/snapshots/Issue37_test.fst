module Issue37_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn set
(n : ref Int32.t)
requires n |-> 'v
ensures n |-> 0l
{
n := 0l;
}

fn call_fib_rec
(n : Int32.t)
returns res:int32
{
let mut cur: Int32.t = witness #_ #_;
(set cur);
(! cur);
}
