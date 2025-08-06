module Issue37_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn set
(n : ( ref Int32.t) )
requires n |-> 'v
ensures n |-> 0l
{
let mut n : (ref Int32.t) = n;
(! n) := 0l;
}

fn call_fib_rec
(n : Int32.t)
returns res:int32
{
let mut n : Int32.t = n;
let mut cur: Int32.t = witness #_ #_;
(set (cur));
(! cur);
}
