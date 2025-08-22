module Issue46_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn get ()
returns (ref Int32.t)
{
let mut p: Int32.t = witness #_ #_;
(p);
}

fn get2
(x : Int32.t)
returns (ref Int32.t)
{
let mut x : Int32.t = x;
(x);
}

[@@expect_failure]
fn get3
(x : Int32.t)
returns res : ref Int32.t
ensures 
exists* x. res |-> x
{
let mut x : Int32.t = x;
(x);
}
