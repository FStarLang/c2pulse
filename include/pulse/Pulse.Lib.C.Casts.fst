module Pulse.Lib.C.Casts
#lang-pulse
open Pulse
include FStar.SizeT
include FStar.Int.Cast

let int32_to_sizet (x: FStar.Int32.t {FStar.Int32.gte x 0l}) =
  let u = int32_to_uint32 x in
  assume (SizeT.fits_u32);
  uint32_to_sizet u