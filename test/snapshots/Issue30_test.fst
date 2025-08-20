module Issue30_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
[@@expect_failure]
fn max_alt
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
let orig_x = x;
let mut result : Int32.t = 0l;
let mut xx : Int32.t = (! (! x));
let mut yy : Int32.t = (! (! y));
with vx. assert x |-> vx;
with vy. assert y |-> vy;
with vxx. assert xx |-> vxx;
with vyy. assert yy |-> vyy;
if((int32_to_bool (bool_to_int32 (Int32.gt (! xx) (! yy)))))
ensures exists* r. (x |-> Frac px vx) ** (y |-> Frac py vy) ** (xx |-> vxx) ** (yy |-> vyy) ** (result |-> r) ** pure (as_int r == max_spec (as_int vxx) (as_int vyy))
{
result := (! xx);
}
else
{
result := (! yy);
};
(! result);
}

fn max_alt2
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy : erased _)
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
let mut result : Int32.t = 0l;
let mut vx : Int32.t = (! (! x));
let mut vy : Int32.t = (! (! y));
if((int32_to_bool (bool_to_int32 (Int32.gt (! vx) (! vy)))))
{
result := (! vx);
}
else
{
result := (! vy);
};
(! result);
}

fn max_alt3
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy : erased _)
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
let mut result : Int32.t = 0l;
if((int32_to_bool (bool_to_int32 (Int32.gt (! (! x)) (! (! y))))))
{
result := (! (! x));
}
else
{
result := (! (! y));
};
(! result);
}

fn max_alt4
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy : erased _)
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
