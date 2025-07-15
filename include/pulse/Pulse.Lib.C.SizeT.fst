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

let eq_int32 (x y:SizeT.t) = if (SizeT.eq x y) then 1l else 0l
let gt_int32 (x y:SizeT.t) = if (SizeT.gt x y) then 1l else 0l
let gte_int32 (x y:SizeT.t) = if (SizeT.gte x y) then 1l else 0l
let lt_int32 (x y:SizeT.t) = if (SizeT.lt x y) then 1l else 0l
let lte_int32 (x y:SizeT.t) = if (SizeT.lte x y) then 1l else 0l