module Pulse.Lib.C.SizeT
include FStar.SizeT
module SZ = FStar.SizeT

let size_t = FStar.SizeT.t
let as_int (x: SZ.t) : int = SZ.v x
let min_size_t = 0
let (+^) = FStar.SizeT.add
let eq (x y: SZ.t) : bool = x=y
instance inhabited_size_t : Pulse.Lib.C.Inhabited.inhabited size_t = {
  witness = 0sz
}