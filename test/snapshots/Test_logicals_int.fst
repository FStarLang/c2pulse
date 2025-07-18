module Test_logicals_int

#lang-pulse

open Pulse
open Pulse.Lib.C



fn f1 ()
returns Int32.t
{
let x0 : Int32.t = 11l;
let mut x : Int32.t = x0;
let y1 : Int32.t = 10l;
let mut y : Int32.t = y1;
let z2 : Int32.t = (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z2;
(! z);
}

fn f2 ()
returns Int32.t
{
let x3 : Int32.t = 10l;
let mut x : Int32.t = x3;
let y4 : Int32.t = 11l;
let mut y : Int32.t = y4;
let z5 : Int32.t = (bool_to_int32 (op_BarBar (int32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z5;
(! z);
}

fn f3 ()
returns Int32.t
{
let x6 : Int32.t = 10l;
let mut x : Int32.t = x6;
let y7 : Int32.t = 20l;
let mut y : Int32.t = y7;
(Int32.add (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! x))))))) (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! y))))))));
}

fn f4 ()
returns Int32.t
{
let x8 : Int32.t = (Int32.sub 0l 10l);
let mut x : Int32.t = x8;
let y9 : Int32.t = 10l;
let mut y : Int32.t = y9;
let z10 : Int32.t = (bool_to_int32 (not (int32_to_bool ((bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))))))));
let mut z : Int32.t = z10;
(! z);
}

fn f5 ()
returns Int32.t
{
let x11 : Int32.t = (Int32.sub 0l 100l);
let mut x : Int32.t = x11;
let y12 : Int32.t = 100l;
let mut y : Int32.t = y12;
let z13 : Int32.t = (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z13;
(! z);
}

[@@expect_failure]
fn f6 ()
returns Int32.t
{
let x14 : UInt32.t = (int32_to_uint32 100l);
let mut x : UInt32.t = x14;
let y15 : Int32.t = (uint32_to_int32 (UInt32.sub 0ul (! x)));
let mut y : Int32.t = y15;
let z16 : Int32.t = (bool_to_int32 (op_AmpAmp (uint32_to_bool (! x)) (int32_to_bool (! y))));
let mut z : Int32.t = z16;
(! z);
}
