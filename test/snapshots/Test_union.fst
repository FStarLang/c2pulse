module Test_union

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type ab = {
a: ref UInt32.t;
b: ref bool;
}

noeq
type ab_spec = 
 | Case_ab_a of UInt32.t
 | Case_ab_b of bool

let ab_pred(u : ref ab) (s : ab_spec) : slprop =
exists* uv. (u |-> uv) **
begin match s with
 | Case_ab_a v -> uv.a |-> v
 | Case_ab_b v -> uv.b |-> v
end

ghost fn ab_explode (x : ref ab) (#s : ab_spec)
requires ab_pred x s
ensures exists* (v : ab). (x |-> v) **
begin match s with
 | Case_ab_a w -> v.a |-> w
 | Case_ab_b w -> v.b |-> w
end
{
unfold ab_pred;
}

ghost
fn ab_recover (x : ref ab) (#s : ab_spec)
requires exists* (v : ab). (x |-> v) **
begin match s with
 | Case_ab_a w -> v.a|-> w
 | Case_ab_b w -> v.b|-> w
end
ensures ab_pred x s
{
fold (ab_pred x s);
}

ghost
fn ab_change_a(x : ref ab)
requires exists* s. ab_pred x s
ensures exists* s. ab_pred x (Case_ab_a s)
{ admit() }

ghost
fn ab_change_b(x : ref ab)
requires exists* s. ab_pred x s
ensures exists* s. ab_pred x (Case_ab_b s)
{ admit() }

ghost
fn ab_is_a(x : ref ab) (#s:_{Case_ab_a? s})
requires ab_pred x s
ensures  ab_pred x (Case_ab_a(Case_ab_a?._0 s))
{ admit() }

ghost
fn ab_is_b(x : ref ab) (#s:_{Case_ab_b? s})
requires ab_pred x s
ensures  ab_pred x (Case_ab_b(Case_ab_b?._0 s))
{ admit() }

open Pulse.Lib.WithPure
fn incr_a
(x : ( ref ab) )
(s:_)
requires with_pure (Case_ab_a? s)
requires ab_pred x s
requires pure (UInt32.fits (UInt32.v (Case_ab_a?._0 s) + 1))
ensures exists* s'. ab_pred x s'
{
let mut x : (ref ab) = x;
ab_is_a (!x);
ab_explode !x;
Mkab?.a (! (! x)) := (UInt32.add (! (! (! x)).a) (int32_to_uint32 1l));
ab_recover !x #(Case_ab_a _);
}

fn set_case_a
(x : ( ref ab) )
(v : UInt32.t)
requires exists* s. ab_pred x s
ensures ab_pred x (Case_ab_a v)
{
let mut x : (ref ab) = x;
let mut v : UInt32.t = v;
ab_change_a !x;
ab_explode !x;
Mkab?.a (! (! x)) := (! v);
ab_recover !x #(Case_ab_a _);
}

noeq
type stru = {
tag: ref Int8.t;
payload: ref ab;
}

[@@erasable]
noeq
type stru_spec = {
tag : Int8.t;
payload : ab_spec

}

[@@pulse_unfold]
let stru_pred ([@@@mkey]x:ref stru) (s:stru_spec) : slprop =
exists* (y: stru). (x |-> y) **
(y.tag |-> s.tag) **
ab_pred y.payload s.payload

assume val stru_spec_default : stru_spec

assume val stru_default (stru_var_spec:stru_spec) : stru

ghost
fn stru_pack (stru_var:ref stru) (#stru_var_spec:stru_spec)
requires stru_var|-> stru_default stru_var_spec
ensures exists* v. stru_pred stru_var v ** pure (v == stru_var_spec)
{ admit() }

ghost
fn stru_unpack (stru_var:ref stru)
requires exists* v. stru_pred stru_var v 
ensures exists* u. stru_var |-> u
{ admit() }

fn stru_alloc ()
returns x:ref stru
ensures freeable x
ensures exists* v. stru_pred x v
{ admit () }

fn stru_free (x:ref stru)
requires freeable x
requires exists* v. stru_pred x v
{ admit() }


ghost fn stru_explode (x:ref stru) (#s:stru_spec)
requires stru_pred x s
ensures exists* (v: stru). (x |-> v) ** (v.tag |-> s.tag) ** 
(v.payload `ab_pred` s.payload)

{unfold stru_pred}


ghost
fn stru_recover (x:ref stru) (#a0 : Int8.t)
(#a1 : ab_spec)

requires exists* (y: stru). (x |-> y) ** 
(y.tag |-> a0) **
(y.payload  `ab_pred` a1)
ensures exists* w. stru_pred x w ** pure (w == {tag = a0;
payload = a1})
{fold stru_pred x ({tag = a0;
payload = a1}) }

[@@pulse_unfold] let stru_ok (u : ref stru) (s : stru_spec) : slprop = stru_pred u s ** pure ( match s.tag with | 0y -> Case_ab_a? s.payload | 1y -> Case_ab_b? s.payload | _ -> False )
fn test_union
(foo : ( ref stru) )
preserves exists* s. stru_ok foo s
{
let mut foo : (ref stru) = foo;
stru_explode (!foo);
if((int32_to_bool (bool_to_int32 (Int32.eq (int8_to_int32 (! (! (! foo)).tag)) 0l))))
{
ab_is_a (! (! foo)).payload;
ab_explode (! (!foo)).payload;
(! (! (! foo)).payload).a := (int32_to_uint32 1l);
ab_recover (! (!foo)).payload #(Case_ab_a _);
stru_recover (!foo);
}
else
{
if((int32_to_bool (bool_to_int32 (Int32.eq (int8_to_int32 (! (! (! foo)).tag)) 1l))))
{
ab_is_b (! (! foo)).payload;
ab_explode (! (!foo)).payload;
(! (! (! foo)).payload).b := (int32_to_bool 0l);
ab_recover (! (!foo)).payload #(Case_ab_b _);
stru_recover (!foo);
}
else
{
unreachable();
};
};
}
