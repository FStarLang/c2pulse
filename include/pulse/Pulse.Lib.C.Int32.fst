module Pulse.Lib.C.Int32
open FStar.Int32
module I32 = FStar.Int32

let int32 = FStar.Int32.t
let as_int (x: I32.t) : int = I32.v x
let fits (op : int -> int -> int) (vx vy : int) : prop =
  FStar.Int32.fits (op vx vy)
let min_int32 = FStar.Int.min_int I32.n
let max_int32 = FStar.Int.max_int I32.n
let (+^) = FStar.Int32.add

instance inhabited_int32 : Pulse.Lib.C.Inhabited.inhabited int32 = {
  witness = I32.zero
}