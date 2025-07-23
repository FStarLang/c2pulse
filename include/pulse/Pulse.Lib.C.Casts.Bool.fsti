module Pulse.Lib.C.Casts.Bool

let bool_to_int64 (b:bool) = if b then 1L else 0L
let bool_to_uint64 (b:bool) = if b then 1UL else 0UL
let bool_to_int32 (b:bool) = if b then 1l else 0l
let bool_to_uint32 (b:bool) = if b then 1ul else 0ul
let bool_to_int16 (b:bool) = if b then 1s else 0s
let bool_to_uint16 (b:bool) = if b then 1us else 0us
let bool_to_int8 (b:bool) = if b then 1y else 0y
let bool_to_uint8 (b:bool) = if b then 1uy else 0uy

let int8_to_bool (i:Int8.t) = Int8.ne i 0y
let int16_to_bool (i:Int16.t) = Int16.ne i 0s
let int32_to_bool (i:Int32.t) = Int32.ne i 0l
let int64_to_bool (i:Int64.t) = Int64.ne i 0L
let uint8_to_bool (i:UInt8.t) = UInt8.ne i 0uy
let uint16_to_bool (i:UInt16.t) = UInt16.ne i 0us
let uint32_to_bool (i:UInt32.t) = UInt32.ne i 0ul
let uint64_to_bool (i:UInt64.t) = UInt64.ne i 0UL