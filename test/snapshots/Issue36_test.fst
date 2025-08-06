module Issue36_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn test
(x : Int32.t)
requires pure (as_int x > 0)
{
let mut x : Int32.t = x;
x := (Int32.sub (! x) 1l);
}
