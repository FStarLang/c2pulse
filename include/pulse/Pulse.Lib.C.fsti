module Pulse.Lib.C
include FStar.Mul
include Pulse.Lib.C.Inhabited
include FStar.SizeT // before Int32 to not shadow fits there
include Pulse.Lib.C.Int32
include Pulse.Lib.C.Ref
include Pulse.Lib.C.Array
include FStar.Int.Cast
include Pulse.Lib.C.Casts
let _Bool = bool
