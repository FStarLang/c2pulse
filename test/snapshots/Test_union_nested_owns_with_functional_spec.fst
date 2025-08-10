module Test_union_nested_owns_with_functional_spec

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



module U32 = Pulse.Lib.C.UInt32 open Pulse.Lib.C.UInt32
noeq
type ab = {
a: ref (ref UInt32.t);
b: ref (ref bool);
}

noeq
type ab_spec = 
 | Case_ab_a of (ref UInt32.t)
 | Case_ab_b of (ref bool)

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

fn incr_a
(x : ( ref ab) )
(#a:erased (ref uint32))
(#v:erased uint32)
preserves ab_pred x (Case_ab_a a)
requires reveal a |-> v
requires pure (U32.fits ( + ) (UInt32.v v) 1)
ensures exists* (u:uint32). (reveal a |-> u) ** pure (U32.as_int u == U32.as_int v + 1)
{
let mut x : (ref ab) = x;
ab_is_a (!x);
ab_explode !x;
(! (! (! x)).a) := (UInt32.add (! (! (! (! x)).a)) (int32_to_uint32 1l));
ab_recover !x #(Case_ab_a _);
}

fn set_case_a
(x : ( ref ab) )
(a : ( ref UInt32.t) )
(#v:erased _)
requires exists* s. ab_pred x s
preserves a |-> v
ensures ab_pred x (Case_ab_a a)
{
let mut x : (ref ab) = x;
let mut a : (ref UInt32.t) = a;
ab_change_a !x;
ab_explode !x;
Mkab?.a (! (! x)) := (! a);
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

let stru_payload (a: ab_spec) (s: either uint32 _Bool) : slprop = match a with | Case_ab_a a -> exists* v. (a |-> v) ** pure (s == Inl v) | Case_ab_b b -> exists* v. (b |-> v) ** pure (s == Inr v) [@@ pulse_unfold] let tag_relation (s:stru_spec): slprop = pure (match s.tag with | 0y -> Case_ab_a? s.payload | 1y -> Case_ab_b? s.payload | _ -> False) [@@ pulse_unfold] let stru_ok_aux (u: ref stru) (ib: either uint32 _Bool) (s:_) : slprop = stru_pred u s ** stru_payload s.payload ib ** tag_relation s [@@ pulse_unfold] let stru_ok (u: ref stru) (ib: either uint32 _Bool) : slprop = exists* s. stru_ok_aux u ib s ghost fn elim_stru_payload_a (a:ab_spec { Case_ab_a? a }) (#ib:_) requires stru_payload a ib ensures exists* v. (Case_ab_a?._0 a |-> v) ** pure (ib == Inl v) { rewrite each a as (Case_ab_a (Case_ab_a?._0 a)); unfold stru_payload; } ghost fn elim_stru_payload_b (a:ab_spec { Case_ab_b? a }) (#ib:_) requires stru_payload a ib ensures exists* v. (Case_ab_b?._0 a |-> v) ** pure (ib == Inr v) { rewrite each a as (Case_ab_b (Case_ab_b?._0 a)); unfold stru_payload; } ghost fn intro_stru_payload_a (a:ab_spec { Case_ab_a? a }) (#v:_) requires (Case_ab_a?._0 a |-> v) ensures stru_payload (Case_ab_a (Case_ab_a?._0 a)) (Inl v) { fold stru_payload (Case_ab_a (Case_ab_a?._0 a)) (Inl v) } ghost fn intro_stru_payload_b (a:ab_spec { Case_ab_b? a }) (#v:_) requires (Case_ab_b?._0 a |-> v) ensures stru_payload (Case_ab_b (Case_ab_b?._0 a)) (Inr v) { fold stru_payload (Case_ab_b (Case_ab_b?._0 a)) (Inr v) } ghost fn intro_stru_ok (u:ref stru) (#s:stru_spec) (#pl:ab_spec) (#ib:_) requires stru_pred u s ** stru_payload pl ib ** pure (s.payload == pl) ** pure ( match s.tag with | 0y -> Case_ab_a? s.payload | 1y -> Case_ab_b? s.payload | _ -> False ) ensures stru_ok u ib { rewrite each pl as s.payload; } ghost fn tag_relation_lemma (y:ref stru) (#ib: either uint32 _Bool) (#s:stru_spec) preserves stru_ok_aux y ib s ensures pure (Inl? ib <==> s.tag==0y) ensures pure (Inr? ib <==> s.tag==1y) { let v = s.tag; if (v = 0y) { ab_is_a _; elim_stru_payload_a _; intro_stru_payload_a _; } else if (v = 1y) { ab_is_b _; elim_stru_payload_b _; intro_stru_payload_b _; } else { unreachable() } }
fn test_union
(foo : ( ref stru) )
(#s:erased (either uint32 _Bool))
requires stru_ok foo s
ensures exists* (t:either uint32 _Bool). stru_ok foo t ** pure (match s with | Inl _ -> t == Inl 1ul | Inr _ -> Inr? t && not (Inr?.v t))
{
let mut foo : (ref stru) = foo;
stru_explode (!foo);
if((int32_to_bool (bool_to_int32 (Int32.eq (int8_to_int32 (! (! (! foo)).tag)) 0l))))
{
ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _;
(! (! (! (! foo)).payload).a) := (int32_to_uint32 1l);
intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _); stru_recover (!foo); intro_stru_ok (!foo);
}
else
{
if((int32_to_bool (bool_to_int32 (Int32.eq (int8_to_int32 (! (! (! foo)).tag)) 1l))))
{
ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _;
(! (! (! (! foo)).payload).b) := (int32_to_bool 0l);
intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _); stru_recover (!foo); intro_stru_ok (!foo);
}
else
{
unreachable();
};
};
}

fn test_union_a
(foo : ( ref stru) )
(#s:erased (either uint32 _Bool) { Inl? s })
requires stru_ok foo s
ensures stru_ok foo (Inl 1ul)
{
let mut foo : (ref stru) = foo;
tag_relation_lemma _; stru_explode (!foo);
ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _;
(! (! (! (! foo)).payload).a) := (int32_to_uint32 1l);
intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _); stru_recover (!foo); intro_stru_ok (!foo);
}

fn test_union_a_alt
(foo : ( ref stru) )
(#s:erased uint32)
requires stru_ok foo (Inl #uint32 s)
ensures stru_ok foo (Inl 1ul)
{
let mut foo : (ref stru) = foo;
tag_relation_lemma _; stru_explode (!foo);
ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _;
(! (! (! (! foo)).payload).a) := (int32_to_uint32 1l);
intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _); stru_recover (!foo); intro_stru_ok (!foo);
}

fn test_union_a_alt2
(foo : ( ref stru) )
(#s:erased uint32)
requires stru_ok foo (Inl #uint32 s)
ensures exists* v. stru_ok foo v ** pure (v == Inl 1ul)
{
let mut foo : (ref stru) = foo;
tag_relation_lemma _; stru_explode (!foo);
ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _;
(! (! (! (! foo)).payload).a) := (int32_to_uint32 1l);
intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _); stru_recover (!foo); intro_stru_ok (!foo);
}

[@@expect_failure [17]]
fn test_union_b_alt
(foo : ( ref stru) )
(#s:erased _Bool)
requires stru_ok foo (Inr #_ #_Bool s)
ensures stru_ok foo (Inr (not s))
{
let mut foo : (ref stru) = foo;
tag_relation_lemma _; stru_explode (!foo);
ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _;
(! (! (! (! foo)).payload).b) := (int32_to_bool (bool_to_int32 (not (! ((! (! (! (! foo)).payload).b))))));
intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _); stru_recover (!foo); intro_stru_ok (!foo);
}

fn test_union_b_alt2
(foo : ( ref stru) )
(#s:erased _Bool)
requires stru_ok foo (Inr #_ #_Bool s)
ensures exists* v. stru_ok foo v ** pure (v == Inr (not s))
{
let mut foo : (ref stru) = foo;
tag_relation_lemma _; stru_explode (!foo);
ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _;
(! (! (! (! foo)).payload).b) := (int32_to_bool (bool_to_int32 (not (! ((! (! (! (! foo)).payload).b))))));
intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _); stru_recover (!foo); intro_stru_ok (!foo);
}

[@pulse_unfold] let ab_cases (uv:ab) (s:ab_spec) : slprop = begin match s with | Case_ab_a v -> uv.a |-> v | Case_ab_b v -> uv.b |-> v end assume val ab_spec_default : ab_spec assume val ab_default (ab_var_spec:ab_spec) : ab instance ab_inhabited : inhabited ab = { witness = (ab_default ab_spec_default) } ghost fn ab_pack (ab_var:ref ab) (#ab_spec:ab_spec) requires ab_var|-> ab_default ab_spec ensures exists* v. ab_pred ab_var v ** pure (v == ab_spec) { admit() } ghost fn ab_unpack (ab_var:ref ab) (#v:ab_spec) requires ab_pred ab_var v ensures exists* ab. (ab_var |-> ab) ** ab_cases ab v { admit() } ghost fn ab_explode_drop (x : ref ab) (#s : ab_spec) requires ab_pred x s ensures exists* (v : ab). (x |-> v) { ab_explode x; drop_ (match s with | Case_ab_a _ -> _ | Case_ab_b _ -> _); }
fn mk_union_a ()
returns x:ref stru
ensures exists* v. stru_ok x v ** pure (v == Inl 0ul) ** freeable x
{
let mut a : (ref UInt32.t) = alloc_ref #UInt32.t ();
(! a) := (uint64_to_uint32 0UL);
let mut ab: ab = witness #_ #_;
ab_pack ab;
(set_case_a (ab) (! a));
let mut x : (ref stru) = stru_alloc ();
stru_explode (!x);
Mkstru?.tag (! (! x)) := (int32_to_int8 0l);
ab_unpack ab; ab_explode_drop (!(!x)).payload;
Mkstru?.payload (! (! x)) := (! ab);
intro_stru_payload_a (Case_ab_a (!a)); ab_recover (!(!x)).payload #(Case_ab_a _); stru_recover (!x); intro_stru_ok (!x); drop_ (freeable (!a));
(! x);
}
