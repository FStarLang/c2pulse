module Stack_allocated_array

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(len : Int32.t)
requires pure (abs (Int32.v len) < 100)
requires pure ((Int32.v len) > 0)
{
let mut len : Int32.t = len;
let mut arr : (array Int32.t) = [| witness #_ #_; 10sz |];
let mut y : Int32.t = (Int32.mul ((Int32.add (! len) 2l)) 2l);
let mut size_expr0 : SizeT.t = int32_to_sizet (! y);
let mut arr1 : (array Int32.t) = [| witness #_ #_; !size_expr0 |];
();
}
