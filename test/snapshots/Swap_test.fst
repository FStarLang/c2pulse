module Swap_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn ref_swap
(r1 : ref Int32.t)
(r2 : ref Int32.t)
requires (r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)
ensures (r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)
{
let mut r1 : (ref Int32.t) = r1;
let mut r2 : (ref Int32.t) = r2;
let tmp0 : Int32.t = (! (! r1));
let mut tmp : Int32.t = tmp0;
(! r1) := (! (! r2));
(! r2) := (! tmp);
}
