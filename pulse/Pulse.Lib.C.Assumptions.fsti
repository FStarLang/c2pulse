module Pulse.Lib.C.Assumptions

open FStar.SizeT { fits_u32, fits_u64 }

// We assume size_t is at least 64 bits.
assume SizeTFitsU64 : fits_u64
assume SizeTFitsU32 : fits_u32

// Whether C assert() is enabled (i.e., NDEBUG is not defined).
// Opaque so the verifier must handle both cases, exposing any
// side effects in assert arguments that would change behavior
// when assertions are disabled.
val func_c2pulse_c_assert_enabled (_:unit) : bool
