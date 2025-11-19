module Pulse.Lib.C.Assumptions

open FStar.SizeT { fits_u32, fits_u64 }

// We assume size_t is at least 64 bits.
assume SizeTFitsU64 : fits_u64
assume SizeTFitsU32 : fits_u32
