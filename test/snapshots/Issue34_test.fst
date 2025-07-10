module Issue34_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn integer_promotion ()
returns b:FStar.UInt32.t
{
let x = 1ul;
if((UInt32.eq x 1ul))
{
0ul;
}
else
{
0ul;
};
}
