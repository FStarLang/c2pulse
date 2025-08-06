module Test_infer_returns

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type _foo = {
a: ref Int32.t;
b: ref Int32.t;
}

[@@erasable]
noeq
type _foo_spec = {
a : Int32.t;
b : Int32.t

}

let _foo_pred ([@@@mkey]x:ref _foo) (s:_foo_spec) : slprop =
exists* (y: _foo). (x |-> y) **
(y.a |-> s.a) **
(y.b |-> s.b)

assume val _foo_spec_default : _foo_spec

assume val _foo_default (_foo_var_spec:_foo_spec) : _foo

ghost
fn _foo_pack (_foo_var:ref _foo) (#_foo_var_spec:_foo_spec)
requires _foo_var|-> _foo_default _foo_var_spec
ensures exists* v. _foo_pred _foo_var v ** pure (v == _foo_var_spec)
{ admit() }

ghost
fn _foo_unpack (_foo_var:ref _foo)
requires exists* v. _foo_pred _foo_var v 
ensures exists* u. _foo_var |-> u
{ admit() }

fn _foo_alloc ()
returns x:ref _foo
ensures freeable x
ensures exists* v. _foo_pred x v
{ admit () }

fn _foo_free (x:ref _foo)
requires freeable x
requires exists* v. _foo_pred x v
{ admit() }


ghost fn _foo_explode (x:ref _foo) (#s:_foo_spec)
requires _foo_pred x s
ensures exists* (v: _foo). (x |-> v) ** (v.a |-> s.a) ** 
(v.b |-> s.b)

{unfold _foo_pred}


ghost
fn _foo_recover (x:ref _foo) (#a0 : Int32.t)
(#a1 : Int32.t)

requires exists* (y: _foo). (x |-> y) ** 
(y.a |-> a0) **
(y.b |-> a1)
ensures exists* w. _foo_pred x w ** pure (w == {a = a0;
b = a1})
{fold _foo_pred x ({a = a0;
b = a1}) }

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

fn foo ()
returns UInt32.t
{
(int32_to_uint32 10l);
}

fn foo1 ()
returns Int64.t
{
(int32_to_int64 10l);
}

fn foo2 ()
returns Int64.t
{
(int32_to_int64 10l);
}

fn foo3 ()
returns (ref Int32.t)
{
let mut x: (ref Int32.t) = witness #_ #_;
(! x);
}

fn foo4 ()
returns (ref UInt32.t)
{
let mut x: (ref UInt32.t) = witness #_ #_;
(! x);
}

fn foo5 ()
{
();
}

[@@expect_failure]
fn foo6 ()
{
((null #_));
}

fn foo7 ()
returns bool
{
(int32_to_bool 0l);
}

fn bar ()
returns Int32.t
{
10l;
}

fn baz ()
returns (ref _foo)
{
(null #_);
}

fn baz1
(x : ( ref ab) )
(s:_)
requires pure (Case_ab_a? s)
returns (ref ab)
{
let mut x : (ref ab) = x;
(! x);
}
