module Pulse.Lib.C.UInt32

include FStar.UInt32
module U32 = FStar.UInt32

let uint32 = U32.t
let as_int (x: U32.t) : int = U32.v x
let fits (op : int -> int -> int) (vx vy : int) : prop =
  U32.fits (op vx vy)
let min_uint32 = FStar.UInt.min_int U32.n
let max_uint32 = FStar.UInt.max_int U32.n
let (+^) = FStar.UInt32.add
// let ( + ) (x y:uint32)
// : Pure uint32 
//   (requires True)
//   (ensures fun z ->
//     if (FStar.UInt.fits (as_int x + as_int y) U32.n)
//     then as_int z == as_int x + as_int y
//     else z == x `U32.add_mod` y)
// = U32.(x `add_mod` y)
// // let add x y = x + y
// let ( * ) (x y:uint32)
// : Pure uint32 
//   (requires True)
//   (ensures fun z ->
//     if (FStar.UInt.fits (as_int x * as_int y) U32.n)
//     then as_int z == as_int x * as_int y
//     else z == x `U32.mul_mod` y)
// = U32.(x `mul_mod` y)
// let (<=) x y = U32.(x <=^ y)

instance inhabited_uint32 : Pulse.Lib.C.Inhabited.inhabited uint32 = {
  witness = U32.zero
}