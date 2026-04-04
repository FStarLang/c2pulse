module Pulse.Lib.C.Casts.Bool

unfold let bool_to_int (b:bool) = if b then 1 else 0
unfold let bool_to_int64 (b:bool) = if b then 1L else 0L
unfold let bool_to_uint64 (b:bool) = if b then 1UL else 0UL
unfold let bool_to_int32 (b:bool) = if b then 1l else 0l
unfold let bool_to_uint32 (b:bool) = if b then 1ul else 0ul
unfold let bool_to_int16 (b:bool) = if b then 1s else 0s
unfold let bool_to_uint16 (b:bool) = if b then 1us else 0us
unfold let bool_to_int8 (b:bool) = if b then 1y else 0y
unfold let bool_to_uint8 (b:bool) = if b then 1uy else 0uy

unfold let int8_to_bool (i:Int8.t) = Int8.ne i 0y
unfold let int16_to_bool (i:Int16.t) = Int16.ne i 0s
unfold let int32_to_bool (i:Int32.t) = Int32.ne i 0l
unfold let int64_to_bool (i:Int64.t) = Int64.ne i 0L
unfold let uint8_to_bool (i:UInt8.t) = UInt8.ne i 0uy
unfold let uint16_to_bool (i:UInt16.t) = UInt16.ne i 0us
unfold let uint32_to_bool (i:UInt32.t) = UInt32.ne i 0ul
unfold let uint64_to_bool (i:UInt64.t) = UInt64.ne i 0UL