module Issue29_test

#lang-pulse

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
fn max
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy : _)
(#px #py : _)
requires x |->Frac px vx
requires y |->Frac py vy
returns n : int32
ensures x |->Frac px vx
ensures y |->Frac py vy
ensures pure(as_int n == max_spec(as_int vx)(as_int vy))
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
if((int32_to_bool (bool_to_int32 (Int32.gt (! (! x)) (! (! y))))))
{
(! (! x));
}
else
{
(! (! y));
};
}
