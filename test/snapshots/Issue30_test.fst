module Issue30_test

#lang-pulse

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
fn max_alt
(x : ref Int32.t)
(y : ref Int32.t)
(#vx #vy : _)
(#px #py : _)
requires x |->Frac px vx
requires y |->Frac py vy
returns n : int32
ensures x |->Frac px vx
ensures y |->Frac py vy
ensures pure(as_int n == max_spec(as_int vx)(as_int vy))
{
let mut result = 0l;
let vx = (! x);
let vy = (! y);
if((Int32.gt vx vy))
ensures exists* r. (x |->Frac px vx) ** (y |->Frac py vy) ** (result |->r) ** pure(as_int r == max_spec(as_int vx)(as_int vy))
{
result := vx;
}
else
{
result := vy;
};
(! result);
}
