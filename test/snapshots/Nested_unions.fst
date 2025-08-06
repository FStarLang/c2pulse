module Nested_unions

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type ab = {
a: ref Int32.t;
b: ref bool;
}

noeq
type ab_spec = 
 | Case_ab_a of Int32.t
 | Case_ab_b of bool

let ab_pred(u : ref ab) (s : ab_spec) : slprop =
exists* uv. (u |-> uv) **
begin match s with
 | Case_ab_a v -> uv.a |-> v
 | Case_ab_b v -> uv.b |-> v
end

fn ab_explode (x : ref ab) (#s : ab_spec)
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

noeq
type nested = {
x: ref ab;
z: ref Int8.t;
}

noeq
type nested_spec = 
 | Case_nested_x of ab
 | Case_nested_z of Int8.t

let nested_pred(u : ref nested) (s : nested_spec) : slprop =
exists* uv. (u |-> uv) **
begin match s with
 | Case_nested_x v -> uv.x |-> v
 | Case_nested_z v -> uv.z |-> v
end

fn nested_explode (x : ref nested) (#s : nested_spec)
requires nested_pred x s
ensures exists* (v : nested). (x |-> v) **
begin match s with
 | Case_nested_x w -> v.x |-> w
 | Case_nested_z w -> v.z |-> w
end
{
unfold nested_pred;
}

ghost
fn nested_recover (x : ref nested) (#s : nested_spec)
requires exists* (v : nested). (x |-> v) **
begin match s with
 | Case_nested_x w -> v.x|-> w
 | Case_nested_z w -> v.z|-> w
end
ensures nested_pred x s
{
fold (nested_pred x s);
}

noeq
type nested2 = {
x: ref ab;
z: ref Int8.t;
strufield: ref stru;
}

noeq
type nested2_spec = 
 | Case_nested2_x of ab
 | Case_nested2_z of Int8.t
 | Case_nested2_strufield of stru

let nested2_pred(u : ref nested2) (s : nested2_spec) : slprop =
exists* uv. (u |-> uv) **
begin match s with
 | Case_nested2_x v -> uv.x |-> v
 | Case_nested2_z v -> uv.z |-> v
 | Case_nested2_strufield v -> uv.strufield |-> v
end

fn nested2_explode (x : ref nested2) (#s : nested2_spec)
requires nested2_pred x s
ensures exists* (v : nested2). (x |-> v) **
begin match s with
 | Case_nested2_x w -> v.x |-> w
 | Case_nested2_z w -> v.z |-> w
 | Case_nested2_strufield w -> v.strufield |-> w
end
{
unfold nested2_pred;
}

ghost
fn nested2_recover (x : ref nested2) (#s : nested2_spec)
requires exists* (v : nested2). (x |-> v) **
begin match s with
 | Case_nested2_x w -> v.x|-> w
 | Case_nested2_z w -> v.z|-> w
 | Case_nested2_strufield w -> v.strufield|-> w
end
ensures nested2_pred x s
{
fold (nested2_pred x s);
}
