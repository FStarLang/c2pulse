module Test_logicals_int

#lang-pulse

open Pulse
open Pulse.Lib.C



fn f1 ()
returns Int32.t
{
let mut x : Int32.t = 11l;
let mut y : Int32.t = 10l;
let mut z : Int32.t = (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))));
(! z);
}

fn f2 ()
returns Int32.t
{
let mut x : Int32.t = 10l;
let mut y : Int32.t = 11l;
let mut z : Int32.t = (bool_to_int32 (op_BarBar (int32_to_bool (! x)) (int32_to_bool (! y))));
(! z);
}

fn f3 ()
returns Int32.t
{
let mut x : Int32.t = 10l;
let mut y : Int32.t = 20l;
(Int32.add (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! x))))))) (bool_to_int32 (not (int32_to_bool (bool_to_int32 (not (int32_to_bool (! y))))))));
}

fn f4 ()
returns Int32.t
{
let mut x : Int32.t = (Int32.sub 0l 10l);
let mut y : Int32.t = 10l;
let mut z : Int32.t = (bool_to_int32 (not (int32_to_bool ((bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))))))));
(! z);
}

fn f5 ()
returns Int32.t
{
let mut x : Int32.t = (Int32.sub 0l 100l);
let mut y : Int32.t = 100l;
let mut z : Int32.t = (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))));
(! z);
}

fn f6 ()
returns Int32.t
{
let mut x : UInt32.t = (int32_to_uint32 100l);
let mut y : Int32.t = (uint32_to_int32 (UInt32.minus (! x)));
let mut z : Int32.t = (bool_to_int32 (op_AmpAmp (uint32_to_bool (! x)) (int32_to_bool (! y))));
(! z);
}
