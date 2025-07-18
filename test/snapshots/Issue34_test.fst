module Issue34_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn integer_promotion ()
returns b:FStar.UInt32.t
{
let x0 : UInt32.t = (int32_to_uint32 1l);
let mut x : UInt32.t = x0;
if((int32_to_bool (bool_to_int32 (UInt32.eq (! x) (int32_to_uint32 1l)))))
{
(int32_to_uint32 0l);
}
else
{
(int32_to_uint32 0l);
};
}
