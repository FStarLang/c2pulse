module Issue65_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type ab_struct = {
a: ref (array UInt8.t);
b: ref (array UInt8.t);
}

[@@erasable]
noeq
type ab_struct_spec = {
a : array UInt8.t;
b : array UInt8.t

}

let ab_struct_relations (s:ab_struct_spec) : prop = 
Pulse.Lib.Array.length s.a == 24 /\
Pulse.Lib.Array.length s.b == 48

[@@pulse_unfold]
let ab_struct_pred ([@@@mkey]x:ref ab_struct) (s:ab_struct_spec) : slprop =
exists* (y: ab_struct). (x |-> y) **
(y.a |-> s.a) **
(y.b |-> s.b)
** pure (ab_struct_relations s)

assume val ab_struct_spec_default : ab_struct_spec

assume val ab_struct_default (ab_struct_var_spec:ab_struct_spec) : ab_struct

ghost
fn ab_struct_pack (ab_struct_var:ref ab_struct) (#ab_struct_var_spec:ab_struct_spec)
requires ab_struct_var|-> ab_struct_default ab_struct_var_spec
ensures exists* v. ab_struct_pred ab_struct_var v ** pure (v == ab_struct_var_spec)
{ admit() }

ghost
fn ab_struct_unpack (ab_struct_var:ref ab_struct)
requires exists* v. ab_struct_pred ab_struct_var v 
ensures exists* u. ab_struct_var |-> u
{ admit() }

fn ab_struct_alloc ()
returns x:ref ab_struct
ensures freeable x
ensures exists* v. ab_struct_pred x v
{ admit () }

fn ab_struct_free (x:ref ab_struct)
requires freeable x
requires exists* v. ab_struct_pred x v
{ admit() }


ghost fn ab_struct_explode (x:ref ab_struct) (#s:ab_struct_spec)
requires ab_struct_pred x s
ensures exists* (v: ab_struct). (x |-> v) ** (v.a |-> s.a) ** 
(v.b |-> s.b)

** pure (ab_struct_relations s)
{unfold ab_struct_pred}


ghost
fn ab_struct_recover (x:ref ab_struct) (#a0 : (array UInt8.t))
(#a1 : (array UInt8.t))

requires exists* (y: ab_struct). (x |-> y) ** 
(y.a |-> a0) **
(y.b |-> a1)** pure (ab_struct_relations {
a = a0;
b = a1})

ensures exists* w. ab_struct_pred x w ** pure (w == {a = a0;
b = a1})
{fold ab_struct_pred x ({a = a0;
b = a1}) }

noeq
type ab_union = {
a: ref (array UInt8.t);
b: ref (array UInt8.t);
}

noeq
type ab_union_spec = 
 | Case_ab_union_a of (array UInt8.t)
 | Case_ab_union_b of (array UInt8.t)

let ab_union_relations (s:ab_union_spec) : prop = 
match s with
| Case_ab_union_a a -> length a == 24
| Case_ab_union_b b -> length b == 48


let ab_union_pred(u : ref ab_union) (s : ab_union_spec) : slprop =
exists* uv. (u |-> uv) **
begin match s with
 | Case_ab_union_a v -> uv.a |-> v
 | Case_ab_union_b v -> uv.b |-> v
end
** pure (ab_union_relations s)

ghost fn ab_union_explode (x : ref ab_union) (#s : ab_union_spec)
requires ab_union_pred x s
ensures exists* (v : ab_union). (x |-> v) **
begin match s with
 | Case_ab_union_a w -> v.a |-> w
 | Case_ab_union_b w -> v.b |-> w
end
** pure (ab_union_relations s)
{
unfold ab_union_pred;
}

ghost
fn ab_union_recover (x : ref ab_union) (#s : ab_union_spec)
requires exists* (v : ab_union). (x |-> v) **
begin match s with
 | Case_ab_union_a w -> v.a|-> w
 | Case_ab_union_b w -> v.b|-> w
end
** pure (ab_union_relations s)
ensures ab_union_pred x s
{
fold (ab_union_pred x s);
}

ghost
fn ab_union_change_a(x : ref ab_union)
requires exists* s. ab_union_pred x s
ensures exists* s. ab_union_pred x (Case_ab_union_a s)
{ admit() }

ghost
fn ab_union_change_b(x : ref ab_union)
requires exists* s. ab_union_pred x s
ensures exists* s. ab_union_pred x (Case_ab_union_b s)
{ admit() }

ghost
fn ab_union_is_a(x : ref ab_union) (#s:_{Case_ab_union_a? s})
requires ab_union_pred x s
ensures  ab_union_pred x (Case_ab_union_a(Case_ab_union_a?._0 s))
{ admit() }

ghost
fn ab_union_is_b(x : ref ab_union) (#s:_{Case_ab_union_b? s})
requires ab_union_pred x s
ensures  ab_union_pred x (Case_ab_union_b(Case_ab_union_b?._0 s))
{ admit() }
