module Pulse.Lib.C.Array
open Pulse
open Pulse.Lib.C.Inhabited
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

fn calloc_array u#a (#a:Type u#a) {| has_zero_default a |} (sz:SizeT.t)
  returns r : array a
  ensures freeable_array r
  ensures A.pts_to r (Seq.create (SizeT.v sz) zero_default)