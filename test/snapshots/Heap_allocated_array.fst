module Heap_allocated_array

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(length : SizeT.t)
requires pure (SizeT.v length < 100)
returns Int32.t
{
let mut length : SizeT.t = length;
let mut a : (array Int32.t) = alloc_array #Int32.t 40sz;
let mut b0 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut size_expr0 : SizeT.t = (uint64_to_sizet (UInt64.mul 4UL (sizet_to_uint64 (! length))));
let mut b : (array Int32.t) = alloc_array #Int32.t !size_expr0;
(free_array #Int32.t (! a));
(free_ref (! b0));
(free_array #Int32.t (! b));
1l;
}
