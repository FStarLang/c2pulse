module Malloc_int_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn new_heap_ref
(v : Int32.t)
returns r:ref int32
ensures r |-> v
ensures freeable r
{
let r = alloc_ref #Int32.t ();
r := v;
r;
}
