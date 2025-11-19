module Pulse.Lib.C.Casts
#lang-pulse
open Pulse

open Pulse.Lib.C.Assumptions

module C = FStar.Int.Cast

let int32_to_sizet (x: Int32.t) =
  let u = C.int32_to_uint32 x in
  SizeT.uint32_to_sizet u

let int64_to_sizet (x: Int64.t) =
  let u = C.int64_to_uint64 x in
  SizeT.uint64_to_sizet u

// NB: The deprecation warning below is spurious, we will
// not overflow the signed int due to our precondition.
let sizet_to_int32 (x: SizeT.t) =
  let u = SizeT.sizet_to_uint32 x in
  C.uint32_to_int32 u

let sizet_to_int64 (x: SizeT.t) =
  let u = SizeT.sizet_to_uint64 x in
  C.uint64_to_int64 u