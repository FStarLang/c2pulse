module Pulse_tutorial_conditionals

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



let max_spec x y = if x < y then y else x
fn max
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy:erased _)
(#px #py:_)
requires x |-> Frac px vx
requires y |-> Frac py vy
returns n:int32
ensures x |-> Frac px vx
ensures y |-> Frac py vy
ensures pure (as_int n == max_spec (as_int vx) (as_int vy))
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

[@@expect_failure]
fn max_alt
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
(#vx #vy:_)
(#px #py:_)
requires x |-> Frac px vx
requires y |-> Frac py vy
returns n:int32
ensures x |-> Frac px vx
ensures y |-> Frac py vy
ensures pure (as_int n == max_spec (as_int vx) (as_int vy))
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
let mut result : Int32.t = 0l;
let mut vx : Int32.t = (! (! x));
let mut vy : Int32.t = (! (! y));
if((int32_to_bool (bool_to_int32 (Int32.gt (! vx) (! vy)))))
ensures exists* r. (x |-> Frac px vx) ** (y |-> Frac py vy) ** (result |-> r) ** pure (as_int r == max_spec (as_int vx) (as_int vy))
{
result := (! vx);
}
else
{
result := (! vy);
};
(! result);
}

fn read_nullable
(r : ( ref Int32.t) )
(#w:option int32)
(#p:_)
requires r |->? Frac p w
returns i:int32
ensures r |->? Frac p w
ensures pure (Some? w ==> Some?.v w == i)
{
let mut r : (ref Int32.t) = r;
if((int32_to_bool (bool_to_int32 (is_null (! r)))))
{
elim_intro_null !r;
0l;
}
else
{
elim_non_null !r;
let mut v : Int32.t = (! (! r));
intro_non_null !r;
(! v);
};
}

fn write_nullable
(r : ( ref Int32.t) )
(v : Int32.t)
(#w:option int32)
requires r |->? w
ensures exists* x. (r |->? x) ** pure (if Some? w then x == Some v else x == w)
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = v;
if((int32_to_bool (bool_to_int32 (is_null (! r)))))
{
elim_intro_null !r;
}
else
{
elim_non_null !r;
(! r) := (! v);
intro_non_null !r;
};
}
