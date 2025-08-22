module Issue42

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type foo = {
next: ref (ref foo);
v: ref Int32.t;
}

[@@erasable]
noeq
type foo_spec = {
next : ( ref foo) ;
v : Int32.t

}

[@@pulse_unfold]
let foo_pred ([@@@mkey]x:ref foo) (s:foo_spec) : slprop =
exists* (y: foo). (x |-> y) **
(y.next |-> s.next) **
(y.v |-> s.v)

assume val foo_spec_default : foo_spec

assume val foo_default (foo_var_spec:foo_spec) : foo

ghost
fn foo_pack (foo_var:ref foo) (#foo_var_spec:foo_spec)
requires foo_var|-> foo_default foo_var_spec
ensures exists* v. foo_pred foo_var v ** pure (v == foo_var_spec)
{ admit() }

ghost
fn foo_unpack (foo_var:ref foo)
requires exists* v. foo_pred foo_var v 
ensures exists* u. foo_var |-> u
{ admit() }

fn foo_alloc ()
returns x:ref foo
ensures freeable x
ensures exists* v. foo_pred x v
{ admit () }

fn foo_free (x:ref foo)
requires freeable x
requires exists* v. foo_pred x v
{ admit() }


ghost fn foo_explode (x:ref foo) (#s:foo_spec)
requires foo_pred x s
ensures exists* (v: foo). (x |-> v) ** (v.next |-> s.next) ** 
(v.v |-> s.v)

{unfold foo_pred}


ghost
fn foo_recover (x:ref foo) (#a0 : (ref foo))
(#a1 : Int32.t)

requires exists* (y: foo). (x |-> y) ** 
(y.next |-> a0) **
(y.v |-> a1)
ensures exists* w. foo_pred x w ** pure (w == {next = a0;
v = a1})
{fold foo_pred x ({next = a0;
v = a1}) }

fn set_next_zero
(p : ( ref foo) )
requires 
exists* v vn. foo_pred p v ** foo_pred v.next vn
ensures 
exists* v vn. foo_pred v.next vn ** foo_pred p v ** pure (vn.v == 0l)
{
let mut p : (ref foo) = p;
let mut pn : (ref foo) = (! (! (! p)).next);
Mkfoo?.v (! (! pn)) := 0l;
foo_recover (!pn);
}

fn set_next_next_zero
(p : ( ref foo) )
requires 
exists* v vn vnn. foo_pred p v ** foo_pred v.next vn ** foo_pred vn.next vnn
ensures 
exists* v vn vnn. foo_pred vn.next vnn ** foo_pred v.next vn ** foo_pred p v ** pure (vnn.v == 0l)
{
let mut p : (ref foo) = p;
let mut pn : (ref foo) = (! (! (! p)).next);
let mut pnn : (ref foo) = (! (! (! pn)).next);
Mkfoo?.v (! (! pnn)) := 0l;
foo_recover (!pnn);
}

fn get_next
(p : ( ref foo) )
returns Int32.t
{
let mut p : (ref foo) = p;
admit();
let mut res : Int32.t = (! (! (! (! (! p)).next)).v);
(! res);
}
