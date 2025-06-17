module Swap_test

#lang-pulse

open Pulse

fn ref_swap
(r1 : ref Int32.t)
(r2 : ref Int32.t)
{
let tmp = (! r1);
r1 := (! r2);
r2 := tmp;

}
