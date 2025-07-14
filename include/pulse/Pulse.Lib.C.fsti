module Pulse.Lib.C
include FStar.Mul
include Pulse.Lib.C.Inhabited
include Pulse.Lib.C.Int32
include Pulse.Lib.C.Ref
include Pulse.Lib.C.Array
let _Bool = bool


let bool_to_int64 (b:bool) = if b then 1L else 0L
let bool_to_uint64 (b:bool) = if b then 1UL else 0UL
let bool_to_int32 (b:bool) = if b then 1l else 0l
let bool_to_uint32 (b:bool) = if b then 1ul else 0ul
let bool_to_int16 (b:bool) = if b then 1s else 0s
let bool_to_uint16 (b:bool) = if b then 1us else 0us
let bool_to_int8 (b:bool) = if b then 1y else 0y
let bool_to_uint8 (b:bool) = if b then 1uy else 0uy

//TODO: Vidush check if this int to bool cast is correct?
let int32_to_bool (i:Int32.t) = if (Int32.gt i 0l) then true else false
