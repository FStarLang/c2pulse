module Pulse.Lib.C
include FStar.Mul
include Pulse.Lib.C.Inhabited
include FStar.SizeT // before Int32 to not shadow fits there
include Pulse.Lib.C.Int32
include Pulse.Lib.C.Ref
include Pulse.Lib.C.Array
include FStar.Int.Cast
include Pulse.Lib.C.Casts
include Pulse.Lib.C.UnaryOps
let _Bool = bool

// We assume size_t is at least 64 bits.
assume SizeTFitsU64 : fits_u64
