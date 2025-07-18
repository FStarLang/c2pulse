module Issue44_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn bar ()
{
let x0 : Int32.t = 1l;
let mut x : Int32.t = x0;
();
}
