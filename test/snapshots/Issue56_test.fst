module Issue56_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo ()
{
let mut x: Int32.t = witness #_ #_;
assert emp;
x := 1l;
}
