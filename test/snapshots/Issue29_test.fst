module Issue29_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
fn max
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy : erased _)
(#px #py : _)
preserves x |->Frac px vx
preserves y |->Frac py vy
returns n : int32
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
