module Issue34_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn integer_promotion ()
returns b:FStar.UInt32.t
{
let x = (int32_to_uint32 1l);
if((UInt32.eq x (int32_to_uint32 1l)))
{
(int32_to_uint32 0l);
}
else
{
(int32_to_uint32 0l);
};
}
