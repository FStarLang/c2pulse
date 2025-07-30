module Issue51_test_2

#lang-pulse

open Pulse
open Pulse.Lib.C



fn neg
(x : UInt32.t)
returns Int32.t
{
let mut x : UInt32.t = x;
(uint32_to_int32 (UInt32.minus (! x)));
}

fn neg_2
(x : UInt64.t)
returns Int64.t
{
let mut x : UInt64.t = x;
(uint64_to_int64 (UInt64.minus (! x)));
}

fn not
(x : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
(bool_to_int32 (not (int32_to_bool (! x))));
}
