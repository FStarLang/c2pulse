module Issue49_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo ()
returns Int32.t
{
let mut x : Int32.t = 1l;
let mut y : (ref Int32.t) = (x);
(! x);
}

fn foo2 ()
returns Int32.t
{
let mut x : Int32.t = 1l;
let mut y : (ref Int32.t) = (x);
(! y) := 2l;
(! x);
}
