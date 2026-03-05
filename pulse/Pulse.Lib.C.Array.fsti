module Pulse.Lib.C.Array
open Pulse
module A = Pulse.Lib.Array

#lang-pulse

val freeable_array (#a:Type) (r:array a) : slprop

fn alloc_array u#a (#a:Type u#a) (sz:SizeT.t)
  returns r : array a
  ensures freeable_array r
  ensures A.pts_to_uninit r (SizeT.v sz)

fn free_array u#a (#a:Type u#a) (r:array a)
  requires A.pts_to_uninit_post r
  requires freeable_array r

fn update_arr_with (#a:Type0) (arr: array a) (idx: SizeT.t) (f: (a -> a))
  requires live arr
  requires with_pure (SizeT.v idx < Seq.length (value_of arr))
  returns _r : unit
  ensures live arr
  ensures with_pure (Seq.length (value_of arr) = old (Seq.length (value_of arr)))