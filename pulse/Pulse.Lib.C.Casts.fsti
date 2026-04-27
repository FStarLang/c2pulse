module Pulse.Lib.C.Casts

open Pulse
#lang-pulse
include Pulse.Lib.C.Casts.Bool
include FStar.Int.Cast

val int32_to_sizet (x: Int32.t) : Pure SizeT.t
  (requires Int32.v x >= 0)
  (ensures fun y -> SizeT.v y == FStar.Int32.v x)

val int64_to_sizet (x: Int64.t) : Pure SizeT.t
  (requires Int64.v x >= 0)
  (ensures fun y -> SizeT.v y == Int64.v x)

val sizet_to_int32 (x: SizeT.t) : Pure Int32.t
  (requires SizeT.v x < pow2 31) // make sure to not overflow the signed int
  (ensures fun y -> Int32.v y == SizeT.v x % pow2 32)

val sizet_to_int64 (x: SizeT.t) : Pure Int64.t
  (requires SizeT.v x < pow2 63) // make sure to not overflow the signed int
  (ensures fun y -> Int64.v y == FStar.SizeT.v x % pow2 64)


// val uint64_to_sizet (x: FStar.UInt64.t {FStar.UInt64.gte x 0UL}) : Pure t
//   (requires True)
//   (ensures fun y -> v y == FStar.UInt64.v x)



// val int64_to_sizet (x: FStar.Int64.t {FStar.Int64.gte x 0L}) : Pure t
//   (requires True)
//   (ensures fun y -> v y == FStar.Int64.v x)


// val uint64_to_sizet (x: FStar.UInt64.t {FStar.UInt64.gte x 0UL}) : Pure t
//   (requires True)
//   (ensures fun y -> v y == FStar.UInt64.v x)