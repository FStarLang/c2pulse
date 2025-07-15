module Pulse.Lib.C.Casts

open Pulse
#lang-pulse
open FStar.Mul
open FStar.Int32
open FStar.Int.Cast
open FStar.SizeT

let b2i (b:bool) = if b then 1l else 0l
let bool_to_int64 (b:bool) = if b then 1L else 0L
let bool_to_uint64 (b:bool) = if b then 1UL else 0UL
let bool_to_int32 (b:bool) = if b then 1l else 0l
let bool_to_uint32 (b:bool) = if b then 1ul else 0ul
let bool_to_int16 (b:bool) = if b then 1s else 0s
let bool_to_uint16 (b:bool) = if b then 1us else 0us
let bool_to_int8 (b:bool) = if b then 1y else 0y
let bool_to_uint8 (b:bool) = if b then 1uy else 0uy

//TODO: Vidush check if this int to bool cast is correct?
let int8_to_bool (i:Int8.t { i == 0y \/ i == 1y }) = if (Int8.gt i 0y) then true else false
let int16_to_bool (i:Int16.t { i == 0s \/ i == 1s }) = if (Int16.gt i 0s) then true else false
let int32_to_bool (i:FStar.Int32.t { i == 0l \/ i == 1l }) = if (FStar.Int32.gt i 0l) then true else false
let int64_to_bool (i:Int64.t { i == 0L \/ i == 1L }) = if (Int64.gt i 0L) then true else false
let uint8_to_bool (i:UInt8.t { i == 0uy \/ i == 1uy } ) = if (UInt8.gt i 0uy) then true else false
let uint16_to_bool (i:UInt16.t { i == 0us \/ i == 1us } ) = if (UInt16.gt i 0us) then true else false
let uint32_to_bool (i:UInt32.t { i == 0ul \/ i == 1ul }) = if (UInt32.gt i 0ul) then true else false
let uint64_to_bool (i:UInt64.t { i == 0UL \/ i == 1UL } ) = if (UInt64.gt i 0UL) then true else false

val int32_to_sizet (x: FStar.Int32.t {FStar.Int32.gte x 0l}) : Pure t
  (requires True)
  (ensures fun y -> v y == FStar.Int32.v x)