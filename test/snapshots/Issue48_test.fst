module Issue48_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo ()
returns Int32.t
{
let mut x: Int32.t = witness #_ #_;
(! x);
}
